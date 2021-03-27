# Serial Port for Flutter

`flutter_serialport` is a simple wrapper around [`libserialport`](https://pub.dev/packages/libserialport),
utilizing Flutter's build system to build and deploy the [C-library](https://sigrok.org/wiki/Libserialport)
under the hood. This package does not provide any additional API, but merely helps to make things work
"out of the box" without the need of manually building and deploying libserialport.

Supported platforms:
- Linux
- macOS
- Windows
- Android

## Usage

To use this package, add `flutter_serialport` as a [dependency in your pubspec.yaml file](https://dart.dev/tools/pub/dependencies).

Unfortunately, this package is not available on [pub.dev](https://pub.dev), because the name
is taken by an old discontinued package. However, it's straight-forward to use the package
directly from Git:

```
flutter_serialport:
  git: https://github.com/jpnurmi/flutter_serialport
```

![screenshot](https://raw.githubusercontent.com/jpnurmi/flutter_serialport/master/doc/images/flutter_serialport.png)
