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
  static Handle<Value> NewInstance(EDBus_Pending *pending);
  static Handle<Object> ToObject(EDBus_Pending *pending);

private:

  static Persistent<Function> constructor;

  DPending(EDBus_Pending *pending_);
  ~DPending();

  static Handle<Value> New(const Arguments& args);
  static Handle<Value> Cancel(const Arguments &args);
  static Handle<Value> OnError(const Arguments &args);
  static Handle<Value> OnComplete(const Arguments &args);
  static Handle<Value> GetObject(const Arguments& args);

  static Handle<Value> Destination(Local<String>, const AccessorInfo& info);
  static Handle<Value> Interface(Local<String>, const AccessorInfo& info);
  static Handle<Value> Method(Local<String>, const AccessorInfo& info);
  static Handle<Value> Path(Local<String>, const AccessorInfo& info);

  EDBus_Connection *conn;
};

}

#endif
