from pymodbus.client import ModbusTcpClient
import time


HOST = "192.168.1.51"
PORT = 502
TIMEOUT = 5.0

def main():
    # En 3.x puedes fijar el Unit ID a nivel de cliente (si quieres)
    client = ModbusTcpClient(host=HOST, port=PORT, timeout=TIMEOUT)  # unit_id opcional
    if not client.connect():
        print(f"❌ No se pudo conectar a {HOST}:{PORT}")
        return

    print("✅ Conectado. Leyendo datos… (Ctrl+C para salir)")
    try:
        while True:
            # 👇 OBLIGATORIO en 3.11.3: usar argumentos con nombre
            rr = client.read_holding_registers(address=0, count=2)
            if rr.isError():
                print("HR error:", rr)
            else:
                temp_c = rr.registers[0] / 10.0
                hum    = rr.registers[1] / 10.0

                rc = client.read_coils(address=0, count=1)
                if rc.isError():
                    print("Coils error:", rc)
                else:
                    coil0 = bool(rc.bits[0])
                    print(f"🌡 {temp_c:0.1f} °C  | COIL0={coil0}")

            time.sleep(0.01)
    except KeyboardInterrupt:
        pass
    finally:
        client.close()
        print("🔌 Conexión cerrada.")

if __name__ == "__main__":
    main()
