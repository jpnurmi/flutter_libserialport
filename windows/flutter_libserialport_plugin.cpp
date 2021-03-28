#include "include/flutter_libserialport/flutter_libserialport_plugin.h"

#include <flutter/plugin_registrar_windows.h>
#include <windows.h>

#include <memory>

namespace {

class FlutterLibserialportPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrarWindows *registrar);

  FlutterLibserialportPlugin();

  virtual ~FlutterLibserialportPlugin();
};

void FlutterLibserialportPlugin::RegisterWithRegistrar(
    flutter::PluginRegistrarWindows *registrar) {
  auto plugin = std::make_unique<FlutterLibserialportPlugin>();
  registrar->AddPlugin(std::move(plugin));
}

FlutterLibserialportPlugin::FlutterLibserialportPlugin() {}

FlutterLibserialportPlugin::~FlutterLibserialportPlugin() {}

}  // namespace

void FlutterLibserialportPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  FlutterLibserialportPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrarWindows>(registrar));
}
