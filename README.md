# Serial Port for Flutter

`flutter_serial_port` is a simple wrapper around [`libserialport`](https://pub.dev/packages/libserialport),
utilizing Flutter's build system to build and deploy the [C-library](https://sigrok.org/wiki/Libserialport)
under the hood. This package does not provide any additional API, but merely helps to make things work
"out of the box" without the need of manually building and deploying libserialport.

Supported platforms:
- Linux
- macOS
- Windows
- Android

## Usage

To use this package, add `flutter_serial_port` as a [dependency in your pubspec.yaml file](https://dart.dev/tools/pub/dependencies).

Unfortunately, this package is not available on [pub.dev](https://pub.dev), because the name
is taken by an old discontinued package. However, it's straight-forward to use the package
directly from Git:

```
flutter_serial_port:
  git: https://github.com/jpnurmi/flutter_serial_port
```

![screenshot](https://raw.githubusercontent.com/jpnurmi/flutter_serial_port/master/doc/images/flutter_serial_port.png)
