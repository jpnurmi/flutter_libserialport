#include "include/flutter_serialport/flutter_serialport_plugin.h"

#include <flutter/plugin_registrar_windows.h>
#include <windows.h>

#include <memory>

namespace {

class FlutterSerialportPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrarWindows *registrar);

  FlutterSerialportPlugin();

  virtual ~FlutterSerialportPlugin();
};

void FlutterSerialportPlugin::RegisterWithRegistrar(
    flutter::PluginRegistrarWindows *registrar) {
  auto plugin = std::make_unique<FlutterSerialportPlugin>();
  registrar->AddPlugin(std::move(plugin));
}

FlutterSerialportPlugin::FlutterSerialportPlugin() {}

FlutterSerialportPlugin::~FlutterSerialportPlugin() {}

}  // namespace

void FlutterSerialportPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  FlutterSerialportPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrarWindows>(registrar));
}
