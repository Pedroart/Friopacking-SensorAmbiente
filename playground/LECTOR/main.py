import asyncio
import struct
from binascii import hexlify
from bleak import BleakScanner

# ========= CONFIG =========
TARGET_COMPANY = 0x10F5  # Friopacking (Bleak usa el companyID como int)
KEY = bytes([
    0xA3, 0x7F, 0x1C, 0xD9, 0x88, 0x4E, 0x21, 0xB6,
    0x59, 0x02, 0xEF, 0xC4, 0x6A, 0x90, 0x13, 0xDD
])

# ========= AES-ECB decrypt + PKCS7 unpad =========
def aes_ecb_decrypt(key: bytes, data: bytes) -> bytes:
    # Requiere: pip install cryptography
    from cryptography.hazmat.primitives.ciphers import Cipher, algorithms, modes
    from cryptography.hazmat.backends import default_backend

    if len(key) != 16:
        raise ValueError("KEY debe ser 16 bytes (AES-128).")
    if len(data) % 16 != 0:
        raise ValueError("Ciphertext debe ser múltiplo de 16 (ECB).")

    cipher = Cipher(algorithms.AES(key), modes.ECB(), backend=default_backend())
    dec = cipher.decryptor()
    return dec.update(data) + dec.finalize()

def pkcs7_unpad(padded: bytes, block_size: int = 16) -> bytes:
    if not padded or (len(padded) % block_size) != 0:
        raise ValueError("Padding inválido (tamaño).")
    pad = padded[-1]
    if pad < 1 or pad > block_size:
        raise ValueError("Padding inválido (valor).")
    if padded[-pad:] != bytes([pad]) * pad:
        raise ValueError("Padding inválido (bytes).")
    return padded[:-pad]

# ========= Parse body =========
def parse_body(body8: bytes) -> dict:
    if len(body8) != 8:
        raise ValueError(f"Body esperado 8 bytes, recibido {len(body8)}")

    # Layout (big-endian para int16):
    # [0] device_id (u8)
    # [1] version_id (u8)
    # [2] flags (u8)
    # [3..4] tmp_x100 (i16 BE)
    # [5..6] cpu_x100 (i16 BE)
    # [7] bat_pct (i8)
    device_id, version_id, flags_u8, tmp_x100, cpu_x100, bat_pct = struct.unpack(">BBBhhb", body8)

    return {
        "device_id": device_id,
        "version_id": version_id,
        "flags_raw": flags_u8,
        "flags": {
            "i2c_fail": bool(flags_u8 & (1 << 0)),
            "bat_low":  bool(flags_u8 & (1 << 1)),
            "tmp_fail": bool(flags_u8 & (1 << 2)),
        },
        "tmp_c": tmp_x100 / 100.0,
        "cpu_c": cpu_x100 / 100.0,
        "tmp_x100": tmp_x100,
        "cpu_x100": cpu_x100,
        "bat_pct": int(bat_pct),
    }

def decode_friosensor_mfg(data: bytes) -> dict:
    # En tu diseño actual, Bleak normalmente te entrega SOLO el payload después del CompanyID.
    # Caso esperado: 16 bytes (cipher).
    # Si te llegaran 18 bytes (head+16), quitamos head.
    cipher = data[2:] if len(data) == 18 else data

    if len(cipher) != 16:
        raise ValueError(f"Cipher esperado 16 bytes, recibido {len(cipher)}")

    padded_plain = aes_ecb_decrypt(KEY, cipher)
    plain = pkcs7_unpad(padded_plain, 16)

    if len(plain) != 8:
        raise ValueError(f"Plain esperado 8 bytes (post-unpad), recibido {len(plain)}")

    parsed = parse_body(plain)

    # Adjunta debug
    parsed["_cipher_hex"] = hexlify(cipher).decode()
    parsed["_padded_plain_hex"] = hexlify(padded_plain).decode()
    parsed["_plain_hex"] = hexlify(plain).decode()
    return parsed

# ========= BLE callback =========
def on_detect(device, adv):
    if TARGET_COMPANY not in (adv.manufacturer_data or {}):
        return

    data = adv.manufacturer_data[TARGET_COMPANY]

    print("========================================")
    print("✅ FRIOSENSOR DETECTADO")
    print(f"ADDR : {device.address}")
    print(f"NAME : {device.name}")
    print(f"RSSI : {adv.rssi} dBm")

    print(f"COMPANY : 0x{TARGET_COMPANY:04X}")
    print(f"RAW DATA: {hexlify(data).decode()} ({len(data)} bytes)")

    try:
        p = decode_friosensor_mfg(data)

        print("---- DECRYPT OK ----")
        print(f"CIPHER   : {p['_cipher_hex']}")
        print(f"PLAIN(8) : {p['_plain_hex']}")

        print("---- FIELDS ----")
        print(f"device_id : {p['device_id']}")
        print(f"version_id: {p['version_id']}")
        print(f"flags     : 0x{p['flags_raw']:02X}  {p['flags']}")
        print(f"TMP117    : {p['tmp_c']:.2f} °C  (x100={p['tmp_x100']})")
        print(f"CPU       : {p['cpu_c']:.2f} °C  (x100={p['cpu_x100']})")
        print(f"BAT       : {p['bat_pct']} %")

    except Exception as e:
        print("❌ DECRYPT/PARSE ERROR:", str(e))

async def main():
    print("🔎 Escaneando BLE (solo Company 0x10F5)... Ctrl+C para salir\n")
    print("💡 Si falta cryptography:  python -m pip install cryptography\n")
    scanner = BleakScanner(on_detect)
    await scanner.start()
    try:
        while True:
            await asyncio.sleep(1)
    finally:
        await scanner.stop()

if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        print("\n⛔ Scan detenido")
