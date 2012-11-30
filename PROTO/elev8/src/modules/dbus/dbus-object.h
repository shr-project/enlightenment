#ifndef DBUS_OBJECT_H
#define DBUS_OBJECT_H

#include "dbus-module.h"
#include "dbus-connection.h"

using namespace v8;

namespace dbus {

class DObject : public ObjectWrap {
public:
  static void Init(Handle<Object> target);
  static Handle<Value> NewInstance(DConnection *conn, const Arguments &args);

  EDBus_Object *GetObject() { return obj; }

private:
  DObject(DConnection *connection, const char *bus, const char *path);
  ~DObject();

  static Persistent<Function> constructor;
  static Handle<Value> New(const Arguments& args);
  static Handle<Value> GetProxy(const Arguments& args);

  EDBus_Object *obj;
};

}

#endif
