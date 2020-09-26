
import 'dart:async';

import 'package:flutter/services.dart';

class FlutterSerialPort {
  static const MethodChannel _channel =
      const MethodChannel('flutter_serial_port');

  static Future<String> get platformVersion async {
    final String version = await _channel.invokeMethod('getPlatformVersion');
    return version;
  }
}
