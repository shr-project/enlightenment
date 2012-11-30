#include <Ecore.h>
#include <EDBus.h>
#include <v8.h>
#include "node_object_wrap.h"

#include "dbus-module.h"
#include "dbus-connection.h"
#include "dbus-object.h"
#include "dbus-proxy.h"
#include "dbus-message.h"
#include "dbus-pending.h"

using namespace v8;

namespace dbus {

int _log_domain = -1;

}

extern "C"
void RegisterModule(Handle<Object> target)
{
  dbus::_log_domain = eina_log_domain_register("dbus", EINA_COLOR_CYAN);
  if (!dbus::_log_domain) {
       ERR("Could not register dbus log domain");
       dbus::_log_domain = EINA_LOG_DOMAIN_GLOBAL;
  }

  INF("Initializing dbus module with log domain %d", dbus::_log_domain);

  ecore_init();
  edbus_init();

  dbus::DConnection::Init(target);
  dbus::DObject::Init(target);
  dbus::DProxy::Init(target);
  dbus::DMessage::Init(target);
  dbus::DPending::Init(target);
}
