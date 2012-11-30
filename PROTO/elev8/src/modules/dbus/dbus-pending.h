#ifndef DBUS_PENDING_H
#define DBUS_PENDING_H

#include "dbus-module.h"

using namespace v8;

namespace dbus {

class DPending : public ObjectWrap {
  EDBus_Pending *pending;
public:
  static void Init(Handle<Object> target);

  EDBus_Connection *GetConnection() { return conn; }

private:
  DPending(EDBus_Pending *pending_);
  ~DPending();

  static Handle<Value> New(const Arguments& args);
  static Handle<Value> GetObject(const Arguments& args);

  EDBus_Connection *conn;
};

}

#endif
