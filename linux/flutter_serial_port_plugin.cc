#include "include/flutter_serial_port/flutter_serial_port_plugin.h"

#include <flutter_linux/flutter_linux.h>
#include <glib.h>

#define FLUTTER_SERIAL_PORT_PLUGIN(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj), flutter_serial_port_plugin_get_type(), \
                              FlutterSerialPortPlugin))

struct _FlutterSerialPortPlugin {
  GObject parent_instance;
};

G_DEFINE_TYPE(FlutterSerialPortPlugin, flutter_serial_port_plugin, g_object_get_type())

static void flutter_serial_port_plugin_class_init(FlutterSerialPortPluginClass* klass) {}

static void flutter_serial_port_plugin_init(FlutterSerialPortPlugin* self) {}

static gchar* g_self_exe() {
  g_autoptr(GError) error = nullptr;
  g_autofree gchar* self_exe = g_file_read_link("/proc/self/exe", &error);
  if (error) {
    g_critical("g_file_read_link: %s", error->message);
  }
  return g_path_get_dirname(self_exe);
}

void flutter_serial_port_plugin_register_with_registrar(FlPluginRegistrar* registrar) {
  FlutterSerialPortPlugin* plugin = FLUTTER_SERIAL_PORT_PLUGIN(
      g_object_new(flutter_serial_port_plugin_get_type(), nullptr));

  g_autofree gchar* self_exe = g_self_exe();
  g_autofree gchar* libserialport_path =
      g_build_filename(self_exe, "lib", "libserialport.so", nullptr);
  setenv("LIBSERIALPORT_PATH", libserialport_path, 0);

  g_object_unref(plugin);
}
