import asyncio
from bleak import BleakScanner, BleakClient

DEVICE_NAME = "FrioTunel-Beta"
UUID_RX = "0000fff1-0000-1000-8000-00805f9b34fb"
UUID_TX = "0000fff2-0000-1000-8000-00805f9b34fb"

# =====================================================
# === Callback para imprimir las notificaciones (TX)
# =====================================================
def handle_notification(sender, data):
    msg = data.decode(errors="ignore").strip()
    print(f"\n📡 {msg}")

# =====================================================
# === Escanear y conectar al dispositivo por nombre
# =====================================================
async def find_device(name: str):
    print(f"🔍 Buscando dispositivo BLE llamado '{name}'...")
    devices = await BleakScanner.discover(timeout=5)
    for d in devices:
        if d.name and name.lower() in d.name.lower():
            print(f"✅ Encontrado: {d.name} [{d.address}]")
            return d.address
    print("❌ Dispositivo no encontrado.")
    return None

# =====================================================
# === Sesión interactiva de comandos BLE
# =====================================================
async def ble_console(address: str):
    async with BleakClient(address) as client:
        print("🔗 Conectado a", address)

        # Iniciar recepción de notificaciones
        await client.start_notify(UUID_TX, handle_notification)

        print("💬 Escribe comandos BLE (ejemplo: TEMP?, BLE=ON, BLE=OFF, IP=192.168.1.50)")
        print("⏎ Presiona ENTER sin texto para salir.\n")

        while True:
            cmd = input("👉 ")
            if not cmd:
                break
            try:
                await client.write_gatt_char(UUID_RX, (cmd + "\n").encode())
            except Exception as e:
                print("⚠️ Error al enviar comando:", e)

        await client.stop_notify(UUID_TX)
    print("🔌 Desconectado.")

# =====================================================
# === Programa principal
# =====================================================
async def main():
    addr = await find_device(DEVICE_NAME)
    if not addr:
        return
    try:
        await ble_console(addr)
    except KeyboardInterrupt:
        print("\n🛑 Cancelado por el usuario")

if __name__ == "__main__":
    asyncio.run(main())
