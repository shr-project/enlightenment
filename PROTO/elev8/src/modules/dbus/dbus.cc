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

static Handle<Value> sigIt(Handle<Value> val, const char *sig)
{
   val->ToObject()->Set(String::NewSymbol("signature"), String::New(sig), DontEnum);
   return val;
}

static Handle<Value> DBoolean(const Arguments& args)
{
   return args[0]->ToBoolean();
}

static Handle<Value> DByte(const Arguments& args)
{
   return sigIt(NumberObject::New(args[0]->ToUint32()->Value()), "y");
}

static Handle<Value> DInt16(const Arguments& args)
{
   return sigIt(NumberObject::New(args[0]->Int32Value()), "n");
}

static Handle<Value> DInt32(const Arguments& args)
{
   return sigIt(NumberObject::New(args[0]->Int32Value()), "i");
}

static Handle<Value> DInt64(const Arguments& args)
{
   return sigIt(NumberObject::New(args[0]->Int32Value()), "x");
}

static Handle<Value> DUint16(const Arguments& args)
{
   return sigIt(NumberObject::New(args[0]->ToUint32()->Value()), "q");
}

static Handle<Value> DUint32(const Arguments& args)
{
   return sigIt(NumberObject::New(args[0]->ToUint32()->Value()), "u");
}

static Handle<Value> DUint64(const Arguments& args)
{
   return sigIt(NumberObject::New(args[0]->ToUint32()->Value()), "t");
}

static Handle<Value> DDouble(const Arguments& args)
{
   return sigIt(NumberObject::New(args[0]->NumberValue()), "d");
}

static Handle<Value> DString(const Arguments& args)
{
   return sigIt(StringObject::New(args[0]->ToString()), "s");
}

static Handle<Value> DArray(const Arguments& args)
{
   unsigned int len = args.Length();
   Handle<Array> array = Array::New(len);
   for (unsigned int i = 0; i < len; i++)
        array->Set(i, args[i]);
   return array;
}

static Handle<Value> DStruct(const Arguments& args)
{
   return sigIt(DArray(args), "r");
}

static Handle<Value> DVariant(const Arguments& args)
{
   return sigIt(DArray(args), "v");
}

extern "C" void RegisterModule(Handle<Object> target);

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

  target->Set(String::NewSymbol("Boolean"), FunctionTemplate::New(DBoolean)->GetFunction());
  target->Set(String::NewSymbol("Byte"), FunctionTemplate::New(DByte)->GetFunction());
  target->Set(String::NewSymbol("Int16"), FunctionTemplate::New(DInt16)->GetFunction());
  target->Set(String::NewSymbol("Int32"), FunctionTemplate::New(DInt32)->GetFunction());
  target->Set(String::NewSymbol("Int64"), FunctionTemplate::New(DInt64)->GetFunction());
  target->Set(String::NewSymbol("UInt16"), FunctionTemplate::New(DUint16)->GetFunction());
  target->Set(String::NewSymbol("UInt32"), FunctionTemplate::New(DUint32)->GetFunction());
  target->Set(String::NewSymbol("UInt64"), FunctionTemplate::New(DUint64)->GetFunction());
  target->Set(String::NewSymbol("Double"), FunctionTemplate::New(DDouble)->GetFunction());
  target->Set(String::NewSymbol("String"), FunctionTemplate::New(DString)->GetFunction());

  target->Set(String::NewSymbol("Array"), FunctionTemplate::New(DArray)->GetFunction());
  target->Set(String::NewSymbol("Struct"), FunctionTemplate::New(DStruct)->GetFunction());
  target->Set(String::NewSymbol("Variant"), FunctionTemplate::New(DVariant)->GetFunction());

  dbus::DConnection::Init(target);
  dbus::DObject::Init(target);
  dbus::DProxy::Init(target);
  dbus::DMessage::Init(target);
  dbus::DPending::Init(target);
}
