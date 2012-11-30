#ifndef DBUS_MODULE_H
#define DBUS_MODULE_H

#include <EDBus.h>
#include <v8.h>
#include "node_object_wrap.h"

#define DBG(...) EINA_LOG_DOM_DBG(dbus::_log_domain, __VA_ARGS__)
#define INF(...) EINA_LOG_DOM_INFO(dbus::_log_domain, __VA_ARGS__)
#define WRN(...) EINA_LOG_DOM_WARN(dbus::_log_domain, __VA_ARGS__)
#define ERR(...) EINA_LOG_DOM_ERR(dbus::_log_domain, __VA_ARGS__)
#define CRT(...) EINA_LOG_DOM_CRITICAL(dbus::_log_domain, __VA_ARGS__)

#define THROW_EXCEPTION(msg) \
  return ThrowException(Exception::Error(String::New(msg)))

namespace dbus {

extern int _log_domain;

}

#endif
