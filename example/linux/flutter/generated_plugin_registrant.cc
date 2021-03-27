//
//  Generated file. Do not edit.
//

#include "generated_plugin_registrant.h"

#include <flutter_serialport/flutter_serialport_plugin.h>

void fl_register_plugins(FlPluginRegistry* registry) {
  g_autoptr(FlPluginRegistrar) flutter_serialport_registrar =
      fl_plugin_registry_get_registrar_for_plugin(registry,
                                                  "FlutterSerialPortPlugin");
  flutter_serialport_plugin_register_with_registrar(
      flutter_serialport_registrar);
}
