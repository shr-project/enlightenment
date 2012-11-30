#include "dbus-connection.h"
#include "dbus-message.h"
#include "dbus-object.h"
#include "dbus-proxy.h"

namespace dbus {

Persistent<Function> DObject::constructor;

DObject::DObject(DConnection *connection, const char *bus, const char *path)
  : obj(edbus_object_get(connection->GetConnection(), bus, path))
{
}

DObject::~DObject()
{
  edbus_object_unref(obj);
}

void DObject::Init(Handle<Object>)
{
  HandleScope scope;

  Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
  tpl->SetClassName(String::NewSymbol("DObject"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  tpl->PrototypeTemplate()->Set(String::NewSymbol("getProxy"),
     FunctionTemplate::New(GetProxy)->GetFunction());

  constructor = Persistent<Function>::New(tpl->GetFunction());
}

Handle<Value> DObject::New(const Arguments& args)
{
  HandleScope scope;
  DConnection *conn = ObjectWrap::Unwrap<DConnection>(args[0]->ToObject());

  if (!args[1]->IsString())
    THROW_EXCEPTION("Missing bus name");
  if (!args[2]->IsString())
    THROW_EXCEPTION("Missing object path");

  DObject *obj = new DObject(conn,
        *String::Utf8Value(args[1]->ToString()),
        *String::Utf8Value(args[2]->ToString()));
  if (!obj->obj)
    THROW_EXCEPTION("Could not get requested object");

  obj->Wrap(args.This());

  return args.This();
}

Handle<Value> DObject::NewInstance(DConnection *conn, const Arguments& args)
{
  HandleScope scope;

  const unsigned argc = 3;
  Handle<Value> argv[argc] = { conn->handle_, args[0], args[1] };

  return scope.Close(constructor->NewInstance(argc, argv));
}

Handle<Value> DObject::GetProxy(const Arguments& args)
{
  DObject *obj = ObjectWrap::Unwrap<DObject>(args.This());
  return DProxy::NewInstance(obj, args);
}

Handle<Value> DObject::Send(const Arguments& args)
{
   if (!DMessage::IsMessage(args[0]))
     THROW_EXCEPTION("Expecting message");
   if (!args[1]->IsFunction())
     THROW_EXCEPTION("Expecting callback function");

   HandleScope scope;
   return Undefined();
}

}
