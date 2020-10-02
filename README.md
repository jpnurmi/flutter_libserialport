# Serial Port for Flutter

[![pub](https://img.shields.io/pub/v/flutter_serial_port.svg)](https://pub.dev/packages/flutter_serial_port)
[![license: MIT](https://img.shields.io/badge/license-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

`flutter_serial_port` is a simple wrapper around [`dart_serial_port`](https://pub.dev/packages/dart_serial_port),
utilizing Flutter's build system to build and deploy [libserialport](https://sigrok.org/wiki/Libserialport)
under the hood. This package does not provide any additional API, but merely helps to make things work
"out of the box" without the need of manually building and deploying libserialport.

Supported platforms:
- Linux
- macOS
- Windows
- Android

![screenshot](https://raw.githubusercontent.com/jpnurmi/flutter_serial_port/master/doc/images/flutter_serial_port.png)

## Usage

To use this package, add `flutter_serial_port` as a [dependency in your pubspec.yaml file](https://dart.dev/tools/pub/dependencies).
