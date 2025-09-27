import 'package:flutter/material.dart';
import 'package:flutter_libserialport/flutter_libserialport.dart';
import 'safe_port_scanner.dart';

/// A safer version of the main example that demonstrates improved
/// error handling for USB UART device disconnection/reconnection scenarios.
void main() => runApp(SafeExampleApp());

class SafeExampleApp extends StatefulWidget {
  @override
  _SafeExampleAppState createState() => _SafeExampleAppState();
}

extension IntToString on int {
  String toHex() => '0x${toRadixString(16)}';
  String toPadded([int width = 3]) => toString().padLeft(width, '0');
  String toTransport() {
    switch (this) {
      case SerialPortTransport.usb:
        return 'USB';
      case SerialPortTransport.bluetooth:
        return 'Bluetooth';
      case SerialPortTransport.native:
        return 'Native';
      default:
        return 'Unknown';
    }
  }
}

class _SafeExampleAppState extends State<SafeExampleApp> {
  var availablePorts = <String>[];
  late SafePortScanner _portScanner;
  bool _isScanning = false;
  String? _lastError;

  @override
  void initState() {
    super.initState();
    
    // Initialize the safer port scanner
    _portScanner = SafePortScanner(
      onPortsChanged: (ports) {
        if (mounted) {
          setState(() {
            availablePorts = ports;
            _lastError = null; // Clear any previous errors
          });
        }
      },
      onError: (error) {
        if (mounted) {
          setState(() {
            _lastError = 'Port scanning error: $error';
            _isScanning = false;
          });
        }
      },
    );
    
    // Perform initial scan
    initPorts();
  }

  @override
  void dispose() {
    _portScanner.dispose();
    super.dispose();
  }

  void initPorts() async {
    setState(() {
      _isScanning = true;
      _lastError = null;
    });
    
    try {
      final ports = await _portScanner.scanPortsOnce();
      if (mounted) {
        setState(() {
          availablePorts = ports;
          _isScanning = false;
        });
      }
    } catch (e) {
      if (mounted) {
        setState(() {
          _lastError = 'Failed to scan ports: $e';
          _isScanning = false;
        });
      }
    }
  }

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: Scaffold(
        appBar: AppBar(
          title: const Text('Safe Flutter Serial Port Example'),
          backgroundColor: Colors.green,
        ),
        body: Column(
          children: [
            // Status and error display
            Container(
              padding: const EdgeInsets.all(16.0),
              child: Column(
                crossAxisAlignment: CrossAxisAlignment.start,
                children: [
                  // Safety improvements info
                  Container(
                    padding: const EdgeInsets.all(12.0),
                    decoration: BoxDecoration(
                      color: Colors.blue.shade50,
                      borderRadius: BorderRadius.circular(8.0),
                      border: Border.all(color: Colors.blue.shade200),
                    ),
                    child: Column(
                      crossAxisAlignment: CrossAxisAlignment.start,
                      children: [
                        Row(
                          children: [
                            Icon(Icons.security, color: Colors.blue.shade700, size: 20),
                            const SizedBox(width: 8),
                            Text(
                              'Safety Improvements Active',
                              style: TextStyle(
                                fontWeight: FontWeight.bold,
                                color: Colors.blue.shade700,
                              ),
                            ),
                          ],
                        ),
                        const SizedBox(height: 8),
                        const Text(
                          '• Better error handling for device disconnection\n'
                          '• Timeout protection prevents hanging\n'
                          '• Buffer overflow protection\n'
                          '• Race condition mitigation',
                          style: TextStyle(fontSize: 12),
                        ),
                      ],
                    ),
                  ),
                  const SizedBox(height: 16),
                  
                  if (_isScanning)
                    const Row(
                      children: [
                        SizedBox(width: 16, height: 16, child: CircularProgressIndicator(strokeWidth: 2)),
                        SizedBox(width: 8),
                        Text('Scanning for ports...'),
                      ],
                    ),
                  if (_lastError != null)
                    Container(
                      padding: const EdgeInsets.all(8.0),
                      decoration: BoxDecoration(
                        color: Colors.red.shade100,
                        borderRadius: BorderRadius.circular(4.0),
                        border: Border.all(color: Colors.red),
                      ),
                      child: Row(
                        children: [
                          const Icon(Icons.error, color: Colors.red, size: 16),
                          const SizedBox(width: 8),
                          Expanded(child: Text(_lastError!, style: const TextStyle(color: Colors.red))),
                        ],
                      ),
                    ),
                  if (availablePorts.isEmpty && !_isScanning && _lastError == null)
                    Container(
                      padding: const EdgeInsets.all(8.0),
                      decoration: BoxDecoration(
                        color: Colors.orange.shade100,
                        borderRadius: BorderRadius.circular(4.0),
                        border: Border.all(color: Colors.orange),
                      ),
                      child: const Row(
                        children: [
                          Icon(Icons.info, color: Colors.orange, size: 16),
                          SizedBox(width: 8),
                          Text('No serial ports found. Try refreshing or connecting a device.'),
                        ],
                      ),
                    ),
                ],
              ),
            ),
            // Port list
            Expanded(
              child: Scrollbar(
                child: ListView(
                  children: [
                    for (final address in availablePorts)
                      Builder(builder: (context) {
                        try {
                          final port = SerialPort(address);
                          return ExpansionTile(
                            title: Text(address),
                            children: [
                              CardListTile('Description', port.description),
                              CardListTile('Transport', port.transport.toTransport()),
                              CardListTile('USB Bus', port.busNumber?.toPadded()),
                              CardListTile('USB Device', port.deviceNumber?.toPadded()),
                              CardListTile('Vendor ID', port.vendorId?.toHex()),
                              CardListTile('Product ID', port.productId?.toHex()),
                              CardListTile('Manufacturer', port.manufacturer),
                              CardListTile('Product Name', port.productName),
                              CardListTile('Serial Number', port.serialNumber),
                              CardListTile('MAC Address', port.macAddress),
                            ],
                          );
                        } catch (e) {
                          // Handle case where port details cannot be retrieved
                          return ListTile(
                            title: Text(address),
                            subtitle: Text('Error retrieving port details: $e'),
                            leading: const Icon(Icons.error, color: Colors.orange),
                          );
                        }
                      }),
                  ],
                ),
              ),
            ),
          ],
        ),
        floatingActionButton: FloatingActionButton(
          backgroundColor: _isScanning ? Colors.grey : Colors.green,
          child: _isScanning 
            ? const SizedBox(width: 24, height: 24, child: CircularProgressIndicator(strokeWidth: 2, color: Colors.white))
            : const Icon(Icons.refresh),
          onPressed: _isScanning ? null : initPorts,
        ),
      ),
    );
  }
}

class CardListTile extends StatelessWidget {
  final String name;
  final String? value;

  CardListTile(this.name, this.value);

  @override
  Widget build(BuildContext context) {
    return Card(
      child: ListTile(
        title: Text(value ?? 'N/A'),
        subtitle: Text(name),
      ),
    );
  }
}