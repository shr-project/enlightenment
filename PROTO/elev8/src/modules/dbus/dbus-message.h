#ifndef DBUS_MESSAGE_H
#define DBUS_MESSAGE_H

#include "dbus-module.h"
#include "dbus-connection.h"

using namespace v8;

namespace dbus {

class DMessage : public ObjectWrap {
public:
  static void Init(Handle<Object> target);
  static Handle<Value> NewInstance(DConnection *conn, const Arguments &args);

  EDBus_Message *GetMessage() { return msg; }

  static bool IsMessage(Handle<Value> v);

private:
  DMessage(EDBus_Message *msg_)
    : msg(msg_) {}
  ~DMessage();

  static Handle<Value> New(const Arguments& args);

  /* Instance */
  static Handle<Value> GetPath(Local<String> prop, const AccessorInfo& info);
  static Handle<Value> GetInterface(Local<String> prop, const AccessorInfo& info);
  static Handle<Value> GetMember(Local<String> prop, const AccessorInfo& info);
  static Handle<Value> GetDestination(Local<String> prop, const AccessorInfo& info);
  static Handle<Value> GetSender(Local<String> prop, const AccessorInfo& info);
  static Handle<Value> GetSignature(Local<String> prop, const AccessorInfo& info);
  static Handle<Value> GetError(Local<String> prop, const AccessorInfo& info);

  /* Constructors */
  static Handle<Value> NewCall(const Arguments& args);
  static Handle<Value> NewError(const Arguments& args);
  static Handle<Value> NewMethodReturn(const Arguments& args);
  static Handle<Value> NewSignal(const Arguments& args);

  EDBus_Message *msg;
  static Persistent<FunctionTemplate> base_template;
};

}

#endif
