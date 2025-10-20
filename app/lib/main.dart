import 'dart:async';
import 'package:flutter/material.dart';
import 'package:flutter_reactive_ble/flutter_reactive_ble.dart';
import 'package:permission_handler/permission_handler.dart';

void main() => runApp(const MyApp());

class MyApp extends StatelessWidget {
  const MyApp({super.key});
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'FrioTunel Config',
      theme: ThemeData(
        brightness: Brightness.dark,
        scaffoldBackgroundColor: Colors.black,
        primaryColor: Colors.white,
        textTheme: const TextTheme(
          bodyMedium: TextStyle(color: Colors.white),
        ),
      ),
      home: const BleConfigPage(),
    );
  }
}

class BleConfigPage extends StatefulWidget {
  const BleConfigPage({super.key});
  @override
  State<BleConfigPage> createState() => _BleConfigPageState();
}

class _BleConfigPageState extends State<BleConfigPage> {
  final _ble = FlutterReactiveBle();
  final serviceUuid = Uuid.parse("0000FFF0-0000-1000-8000-00805F9B34FB");
  final rxUuid = Uuid.parse("0000FFF1-0000-1000-8000-00805F9B34FB");
  final txUuid = Uuid.parse("0000FFF2-0000-1000-8000-00805F9B34FB");

  StreamSubscription<DiscoveredDevice>? scanSub;
  StreamSubscription<List<int>>? notifySub;
  Timer? tempTimer;

  List<DiscoveredDevice> foundDevices = [];
  DiscoveredDevice? selectedDevice;

  String ip = '';
  String gw = '';
  String mask = '';
  String avg = '';
  String temp = '--.--';

  final ipCtrl = TextEditingController();
  final gwCtrl = TextEditingController();
  final maskCtrl = TextEditingController();
  final avgCtrl = TextEditingController();

  // === Conversor HEX → IP ===
  String hexToIp(String hex) {
    if (hex.length != 8) return "??.??.??.??";
    try {
      final bytes = [
        int.parse(hex.substring(0, 2), radix: 16),
        int.parse(hex.substring(2, 4), radix: 16),
        int.parse(hex.substring(4, 6), radix: 16),
        int.parse(hex.substring(6, 8), radix: 16),
      ];
      return bytes.join('.');
    } catch (e) {
      return "??.??.??.??";
    }
  }

  Future<void> _requestPermissions() async {
    await [
      Permission.bluetoothScan,
      Permission.bluetoothConnect,
      Permission.location,
    ].request();
  }

  Future<void> _scanDevices() async {
    await _requestPermissions();
    setState(() {
      foundDevices.clear();
      selectedDevice = null;
      ip = gw = mask = avg = '';
      temp = '--.--';
    });

    scanSub?.cancel();
    scanSub = _ble.scanForDevices(withServices: []).listen((d) {
      if (d.name.startsWith("FrioTunel") &&
          !foundDevices.any((x) => x.id == d.id)) {
        setState(() => foundDevices.add(d));
      }
    });
  }

  Future<void> _connectToDevice(DiscoveredDevice device) async {
    await scanSub?.cancel();
    setState(() {
      selectedDevice = device;
      ip = gw = mask = avg = '';
      temp = '--.--';
    });

    _ble.connectToDevice(id: device.id).listen((c) {
      if (c.connectionState == DeviceConnectionState.connected) {
        _listenNotifications(device.id);
        Future.delayed(const Duration(milliseconds: 300), _getAllInfo);
        _startTempPolling();
      }
    });
  }

  void _listenNotifications(String id) {
    final tx = QualifiedCharacteristic(
        serviceId: serviceUuid, characteristicId: txUuid, deviceId: id);

    notifySub?.cancel();
    notifySub = _ble.subscribeToCharacteristic(tx).listen((data) {
      final msg = String.fromCharCodes(data).trim();
      debugPrint("BLE RX: $msg");

      if (msg.startsWith("IP=")) {
        final ipStr = hexToIp(msg.substring(3).trim());
        setState(() {
          ip = ipStr;
          ipCtrl.text = ipStr;
        });
      } else if (msg.startsWith("GW=")) {
        final gwStr = hexToIp(msg.substring(3).trim());
        setState(() {
          gw = gwStr;
          gwCtrl.text = gwStr;
        });
      } else if (msg.startsWith("MASK=")) {
        final maskStr = hexToIp(msg.substring(5).trim());
        setState(() {
          mask = maskStr;
          maskCtrl.text = maskStr;
        });
      } else if (msg.startsWith("AVG=")) {
        setState(() {
          avg = msg.substring(4);
          avgCtrl.text = avg;
        });
      } else if (msg.startsWith("TEMP=")) {
        setState(() {
          temp = msg.substring(5).trim();
        });
      } else if (msg.startsWith("OK")) {
        _showSnack("✅ ${msg.trim()}", Colors.green);
        _getAllInfo();
      } else if (msg.startsWith("ERR")) {
        _showSnack("❌ ${msg.trim()}", Colors.redAccent);
      }
    });
  }

  Future<void> _send(String cmd) async {
    if (selectedDevice == null) return;
    final rx = QualifiedCharacteristic(
        serviceId: serviceUuid,
        characteristicId: rxUuid,
        deviceId: selectedDevice!.id);
    await _ble.writeCharacteristicWithResponse(rx, value: cmd.codeUnits);
  }

  Future<void> _getAllInfo() async {
    await _send("IP?=");
    await Future.delayed(const Duration(milliseconds: 200));
    await _send("GW?=");
    await Future.delayed(const Duration(milliseconds: 200));
    await _send("MASK?=");
    await Future.delayed(const Duration(milliseconds: 200));
    await _send("AVG?=");
  }

  void _saveIP() => _send("IP=${ipCtrl.text}");
  void _saveGW() => _send("GW=${gwCtrl.text}");
  void _saveMASK() => _send("MASK=${maskCtrl.text}");
  void _saveAVG() => _send("AVG=${avgCtrl.text}");
  void _reset() => _send("RESET");

  void _startTempPolling() {
    tempTimer?.cancel();
    tempTimer = Timer.periodic(const Duration(seconds: 1), (_) {
      _send("TEMP?");
    });
  }

  void _stopTempPolling() {
    tempTimer?.cancel();
    tempTimer = null;
  }

  void _showSnack(String text, Color color) {
    ScaffoldMessenger.of(context).showSnackBar(
      SnackBar(
        content: Text(text, style: const TextStyle(color: Colors.white)),
        backgroundColor: color,
        duration: const Duration(seconds: 2),
      ),
    );
  }

  @override
  void dispose() {
    scanSub?.cancel();
    notifySub?.cancel();
    _stopTempPolling();
    super.dispose();
  }

  Widget _buildDeviceList() {
    return Column(
      children: [
        ElevatedButton.icon(
          icon: const Icon(Icons.search),
          label: const Text("Buscar dispositivos FrioTunel"),
          onPressed: _scanDevices,
          style: ElevatedButton.styleFrom(
              backgroundColor: Colors.tealAccent[700],
              foregroundColor: Colors.white),
        ),
        const SizedBox(height: 10),
        if (foundDevices.isEmpty)
          const Text("No se encontraron dispositivos FrioTunel."),
        ...foundDevices.map((d) => Card(
              color: Colors.grey[900],
              child: ListTile(
                title: Text(d.name, style: const TextStyle(color: Colors.white)),
                subtitle:
                    Text(d.id, style: const TextStyle(color: Colors.white70)),
                onTap: () => _connectToDevice(d),
              ),
            )),
      ],
    );
  }

  // === NUEVA CARD DE TEMPERATURA ===
  Widget _buildTempCard() {
    return Card(
      color: Colors.grey[900],
      margin: const EdgeInsets.symmetric(vertical: 10),
      child: Padding(
        padding: const EdgeInsets.all(20),
        child: Column(
          children: [
            const Text("Temperatura actual",
                style: TextStyle(fontSize: 16, color: Colors.white70)),
            Text("$temp °C",
                style: const TextStyle(
                    fontSize: 48,
                    fontWeight: FontWeight.bold,
                    color: Colors.tealAccent)),
          ],
        ),
      ),
    );
  }

  Widget _buildField(
      String label, TextEditingController ctrl, VoidCallback onSave,
      {String? hint}) {
    return Padding(
      padding: const EdgeInsets.symmetric(vertical: 6),
      child: Row(
        children: [
          Expanded(
            flex: 3,
            child: TextField(
              controller: ctrl,
              style: const TextStyle(color: Colors.white),
              keyboardType: TextInputType.number,
              decoration: InputDecoration(
                labelText: label,
                hintText: hint,
                labelStyle: const TextStyle(color: Colors.white70),
                filled: true,
                fillColor: Colors.grey[850],
                border: OutlineInputBorder(
                    borderRadius: BorderRadius.circular(8),
                    borderSide: BorderSide.none),
              ),
            ),
          ),
          const SizedBox(width: 8),
          ElevatedButton(
            onPressed: onSave,
            style: ElevatedButton.styleFrom(
              backgroundColor: Colors.tealAccent[700],
              foregroundColor: Colors.white,
            ),
            child: const Text("Guardar"),
          )
        ],
      ),
    );
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text("Configuración FrioTunel"),
        backgroundColor: Colors.black,
      ),
      body: Padding(
        padding: const EdgeInsets.all(16),
        child: SingleChildScrollView(
          child: Column(
            children: [
              if (selectedDevice == null) _buildDeviceList(),
              if (selectedDevice != null) ...[
                Text("Conectado a: ${selectedDevice!.name}",
                    style: const TextStyle(
                        fontSize: 18, fontWeight: FontWeight.bold)),
                _buildTempCard(),
                _buildField("IP", ipCtrl, _saveIP),
                _buildField("Gateway", gwCtrl, _saveGW),
                _buildField("Máscara", maskCtrl, _saveMASK),
                _buildField("AVG (0–4)", avgCtrl, _saveAVG,
                    hint: "Nivel de promedio"),
                const SizedBox(height: 12),
                Row(
                  mainAxisAlignment: MainAxisAlignment.spaceAround,
                  children: [
                    ElevatedButton.icon(
                      icon: const Icon(Icons.refresh),
                      label: const Text("Actualizar"),
                      style: ElevatedButton.styleFrom(
                          backgroundColor: Colors.blueGrey[700]),
                      onPressed: _getAllInfo,
                    ),
                    ElevatedButton.icon(
                      icon: const Icon(Icons.restart_alt),
                      label: const Text("Reiniciar"),
                      style: ElevatedButton.styleFrom(
                          backgroundColor: Colors.redAccent),
                      onPressed: _reset,
                    ),
                  ],
                ),
              ],
            ],
          ),
        ),
      ),
    );
  }
}
