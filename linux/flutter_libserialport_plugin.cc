#include "include/flutter_libserialport/flutter_libserialport_plugin.h"

#include <flutter_linux/flutter_linux.h>
#include <glib.h>

#define FLUTTER_LIBSERIALPORT_PLUGIN(obj)                                     \
  (G_TYPE_CHECK_INSTANCE_CAST((obj), flutter_libserialport_plugin_get_type(), \
                              FlutterLibserialportPlugin))

struct _FlutterLibserialportPlugin {
  GObject parent_instance;
};

G_DEFINE_TYPE(FlutterLibserialportPlugin, flutter_libserialport_plugin,
              g_object_get_type())

static void flutter_libserialport_plugin_class_init(
    FlutterLibserialportPluginClass* klass) {}

static void flutter_libserialport_plugin_init(
    FlutterLibserialportPlugin* self) {}

static gchar* g_self_exe() {
  g_autoptr(GError) error = nullptr;
  g_autofree gchar* self_exe = g_file_read_link("/proc/self/exe", &error);
  if (error) {
    g_critical("g_file_read_link: %s", error->message);
  }
  return g_path_get_dirname(self_exe);
}

void flutter_libserialport_plugin_register_with_registrar(
    FlPluginRegistrar* registrar) {
  FlutterLibserialportPlugin* plugin = FLUTTER_LIBSERIALPORT_PLUGIN(
      g_object_new(flutter_libserialport_plugin_get_type(), nullptr));

  g_autofree gchar* self_exe = g_self_exe();
  g_autofree gchar* libserialport_path =
      g_build_filename(self_exe, "lib", "libserialport.so", nullptr);
  g_setenv("LIBSERIALPORT_PATH", libserialport_path, 0);

  g_object_unref(plugin);
}
