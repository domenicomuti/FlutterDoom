//
//  Generated file. Do not edit.
//

// clang-format off

#include "generated_plugin_registrant.h"

#include <pointer_lock/pointer_lock_plugin.h>

void fl_register_plugins(FlPluginRegistry* registry) {
  g_autoptr(FlPluginRegistrar) pointer_lock_registrar =
      fl_plugin_registry_get_registrar_for_plugin(registry, "PointerLockPlugin");
  pointer_lock_plugin_register_with_registrar(pointer_lock_registrar);
}
