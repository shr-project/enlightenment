#include "dbus-connection.h"
#include "dbus-object.h"
#include "dbus-proxy.h"

namespace dbus {

Persistent<Function> DProxy::constructor;

DProxy::DProxy(DObject *obj, const char *iface)
  : proxy(edbus_proxy_get(obj->GetObject(), iface))
{
}

DProxy::~DProxy()
{
  edbus_proxy_unref(proxy);
}

void DProxy::Init(Handle<Object>)
{
  HandleScope scope;

  Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
  tpl->SetClassName(String::NewSymbol("DProxy"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  tpl->PrototypeTemplate()->Set(String::NewSymbol("addSignalHandler"),
     FunctionTemplate::New(AddSignalHandler)->GetFunction());
  tpl->PrototypeTemplate()->Set(String::NewSymbol("removeSignalHandler"),
     FunctionTemplate::New(RemoveSignalHandler)->GetFunction());
  tpl->PrototypeTemplate()->Set(String::NewSymbol("call"),
     FunctionTemplate::New(Call)->GetFunction());
  tpl->PrototypeTemplate()->Set(String::NewSymbol("send"),
     FunctionTemplate::New(Send)->GetFunction());

  constructor = Persistent<Function>::New(tpl->GetFunction());
}

Handle<Value> DProxy::New(const Arguments& args)
{
  HandleScope scope;
  DObject *obj = ObjectWrap::Unwrap<DObject>(args[0]->ToObject());

  if (!args[1]->IsString())
    THROW_EXCEPTION("Missing interface name");

  DProxy *proxy = new DProxy(obj, *String::Utf8Value(args[1]->ToString()));
  if (!proxy->proxy)
    THROW_EXCEPTION("Could not get requested proxy");

  proxy->Wrap(args.This());

  return args.This();
}

Handle<Value> DProxy::NewInstance(DObject *obj, const Arguments& args)
{
  HandleScope scope;

  const unsigned argc = 2;
  Handle<Value> argv[argc] = { obj->handle_, args[0] };

  return scope.Close(constructor->NewInstance(argc, argv));
}

struct WrappedSignalHandler {
public:
  WrappedSignalHandler(DProxy *proxy_, const char *signal_name_, Handle<Value> cb_, Handle<Value> data_)
    : cb(cb_)
    , data(data_)
    , sh(edbus_proxy_signal_handler_add(proxy_->GetProxy(),
           signal_name_,
           WrappedSignalHandler::Invoke,
           this)) {}

  ~WrappedSignalHandler();

private:
  Handle<Value> cb;
  Handle<Value> data;
  EDBus_Signal_Handler *sh;

  static void Invoke(void *data, const EDBus_Message *msg);
};

void WrappedSignalHandler::Invoke(void *data, const EDBus_Message *)
{
  HandleScope scope;
  WrappedSignalHandler *self = static_cast<WrappedSignalHandler *>(data);
  Handle<Function> callback(Function::Cast(*self->cb));

  const unsigned argc = 2;
  /* FIXME: Convert msg to js */
  Handle<Value> argv[argc] = { self->data, Undefined() };
  callback->Call(Context::GetCurrent()->Global(), argc, argv);
}

WrappedSignalHandler::~WrappedSignalHandler()
{
  edbus_signal_handler_unref(sh);
}

Handle<Value> DProxy::AddSignalHandler(const Arguments& args)
{
  HandleScope scope;
  DProxy *self = ObjectWrap::Unwrap<DProxy>(args.This());

  if (!args[0]->IsString())
    THROW_EXCEPTION("Expecting signal name");
  if (!args[1]->IsFunction())
    THROW_EXCEPTION("Expecting callback");

  return scope.Close(External::Wrap(new WrappedSignalHandler(self,
      *String::Utf8Value(args[0]), args[1], args[2])));
}

Handle<Value> DProxy::RemoveSignalHandler(const Arguments& args)
{
  delete static_cast<WrappedSignalHandler *>(External::Unwrap(args[0]));
  return Undefined();
}

Handle<Value> DProxy::Call(const Arguments&)
{
  return Undefined();
}

struct WrappedProxyMessage {
public:
  WrappedProxyMessage(DProxy *proxy_, DMessage *msg_, Handle<Value> cb_, Handle<Value> data_)
    : cb(cb_)
    , data(data_)
    , pending(edbus_proxy_send(proxy_->GetProxy(),
          msg_->GetMessage(),
          WrappedProxyMessage::Invoke,
          this,
          -1)) {}

  ~WrappedProxyMessage();

private:
  Handle<Value> cb;
  Handle<Value> data;
  EDBus_Pending *pending;

  static void Invoke(void *data, const EDBus_Message *msg, EDBus_Pending *pending);
};

WrappedProxyMessage::~WrappedProxyMessage()
{
  edbus_pending_unref(pending);
}

void WrappedProxyMessage::Invoke(void *data, const EDBus_Message *msg, EDBus_Pending *)
{
  HandleScope scope;
  WrappedProxyMessage *self = static_cast<WrappedProxyMessage *>(data);
  Handle<Function> callback(Function::Cast(*self->cb));

  const unsigned argc = 2;
  /* FIXME: Convert msg to js */
  Handle<Value> argv[argc] = { self->data, Undefined() };
  callback->Call(Context::GetCurrent()->Global(), argc, argv);
}

Handle<Value> DProxy::Send(const Arguments& args)
{
  if (!DMessage::IsMessage(args[0]))
    THROW_EXCEPTION("Expecting message");
  if (!args[1]->IsFunction())
    THROW_EXCEPTION("Expecting callback function");

  HandleScope scope;
  return scope.Close(External::Wrap(new WrappedProxyMessage(self,
    ObjectWrap::Unwrap<DMessage>(args[0]->ToObject()),
    args[1], args[2])));
}

}
