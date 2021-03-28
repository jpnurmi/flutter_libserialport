# Serial Port for Flutter

`flutter_libserialport` is a simple wrapper around [`libserialport`](https://pub.dev/packages/libserialport),
utilizing Flutter's build system to build and deploy the [C-library](https://sigrok.org/wiki/Libserialport)
under the hood. This package does not provide any additional API, but merely helps to make things work
"out of the box" without the need of manually building and deploying libserialport.

Supported platforms:
- Linux
- macOS
- Windows
- Android

## Usage

To use this package, add `flutter_libserialport` as a [dependency in your pubspec.yaml file](https://dart.dev/tools/pub/dependencies).

![screenshot](https://raw.githubusercontent.com/jpnurmi/flutter_libserialport/master/doc/images/flutter_libserialport.png)
