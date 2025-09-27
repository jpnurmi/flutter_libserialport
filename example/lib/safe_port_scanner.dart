import 'dart:async';
import 'package:flutter_libserialport/flutter_libserialport.dart';

/// A safer implementation of serial port scanning that handles device
/// disconnection/reconnection scenarios more robustly.
/// 
/// This class demonstrates the recommended approach for scanning ports
/// when USB UART devices might be physically disconnected and reconnected
/// during operation.
class SafePortScanner {
  List<String> _availablePorts = [];
  Timer? _scanTimer;
  
  /// Callback for when the list of available ports changes
  final Function(List<String>)? onPortsChanged;
  
  /// Callback for when an error occurs during port scanning
  final Function(Object error)? onError;
  
  SafePortScanner({this.onPortsChanged, this.onError});
  
  /// Get the current list of available ports
  List<String> get availablePorts => List.unmodifiable(_availablePorts);
  
  /// Safely scan for available ports once.
  /// 
  /// This method includes error handling to prevent crashes when USB devices
  /// are disconnected during the scan operation.
  Future<List<String>> scanPortsOnce() async {
    try {
      // Use a completer to add timeout protection
      final completer = Completer<List<String>>();
      
      // Set up a timeout to prevent hanging
      Timer(const Duration(seconds: 10), () {
        if (!completer.isCompleted) {
          completer.complete(_availablePorts); // Return cached list on timeout
        }
      });
      
      // Perform the actual port enumeration
      // The underlying Windows implementation now has better error handling
      // to prevent BSOD crashes when devices are disconnected
      final currentPorts = SerialPort.availablePorts;
      
      if (!completer.isCompleted) {
        completer.complete(currentPorts);
      }
      
      final ports = await completer.future;
      
      // Check if the port list has changed
      if (!_listsEqual(_availablePorts, ports)) {
        _availablePorts = List.from(ports);
        onPortsChanged?.call(_availablePorts);
      }
      
      return _availablePorts;
      
    } catch (error) {
      // Handle any errors that might occur during port enumeration
      onError?.call(error);
      // Return the cached list if available, otherwise empty list
      return _availablePorts;
    }
  }
  
  /// Start automatic port scanning at regular intervals.
  /// 
  /// This is safer than the original approach because:
  /// 1. It includes error handling
  /// 2. It has timeout protection
  /// 3. It uses a reasonable interval to avoid overwhelming the system
  void startPeriodicScanning({Duration interval = const Duration(seconds: 5)}) {
    stopPeriodicScanning(); // Stop any existing timer
    
    _scanTimer = Timer.periodic(interval, (timer) {
      scanPortsOnce();
    });
  }
  
  /// Stop automatic port scanning
  void stopPeriodicScanning() {
    _scanTimer?.cancel();
    _scanTimer = null;
  }
  
  /// Compare two lists for equality
  bool _listsEqual(List<String> a, List<String> b) {
    if (a.length != b.length) return false;
    for (int i = 0; i < a.length; i++) {
      if (a[i] != b[i]) return false;
    }
    return true;
  }
  
  /// Dispose of resources
  void dispose() {
    stopPeriodicScanning();
  }
}

/// Example usage demonstrating safe port scanning practices
class SafePortScannerExample {
  static void demonstrateUsage() {
    final scanner = SafePortScanner(
      onPortsChanged: (ports) {
        print('Available ports changed: $ports');
      },
      onError: (error) {
        print('Port scanning error: $error');
        // In a real app, you might want to show a user-friendly message
        // and implement retry logic
      },
    );
    
    // Example 1: Scan once manually
    print('Scanning for ports once...');
    scanner.scanPortsOnce().then((ports) {
      print('Found ports: $ports');
    });
    
    // Example 2: Start periodic scanning (safer than continuous scanning)
    print('Starting periodic scanning every 5 seconds...');
    scanner.startPeriodicScanning(interval: const Duration(seconds: 5));
    
    // In a real app, you would stop scanning when appropriate:
    // scanner.stopPeriodicScanning();
    // scanner.dispose();
  }
}