#include "include/flutter_serialport/flutter_serialport_plugin.h"

#include <flutter/plugin_registrar_windows.h>
#include <windows.h>

#include <memory>

namespace {

class FlutterSerialPortPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrarWindows *registrar);

  FlutterSerialPortPlugin();

  virtual ~FlutterSerialPortPlugin();
};

void FlutterSerialPortPlugin::RegisterWithRegistrar(
    flutter::PluginRegistrarWindows *registrar) {
  auto plugin = std::make_unique<FlutterSerialPortPlugin>();
  registrar->AddPlugin(std::move(plugin));
}

FlutterSerialPortPlugin::FlutterSerialPortPlugin() {}

FlutterSerialPortPlugin::~FlutterSerialPortPlugin() {}

}  // namespace

void FlutterSerialPortPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  FlutterSerialPortPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrarWindows>(registrar));
}
