#include "dbus-connection.h"
#include "dbus-message.h"
#include "dbus-pending.h"
#include "dbus-object.h"
#include "dbus-proxy.h"

namespace dbus {

Persistent<Function> DProxy::constructor;

static Handle<Value> einaValueToValue(Eina_Value *value);

DProxy::DProxy(DObject *_obj, const char *_iface)
  : proxy(edbus_proxy_get(_obj->GetObject(), _iface))
{
   obj = _obj;
   iface = strdup(_iface);
}

DProxy::~DProxy()
{
  edbus_proxy_unref(proxy);
  free(iface);
}

void DProxy::Init(Handle<Object>)
{
  HandleScope scope;

  Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
  tpl->SetClassName(String::NewSymbol("DProxy"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  Local<ObjectTemplate> proto_t = tpl->PrototypeTemplate();

  proto_t->Set(String::NewSymbol("addSignalHandler"),
     FunctionTemplate::New(AddSignalHandler)->GetFunction());
  proto_t->Set(String::NewSymbol("removeSignalHandler"),
     FunctionTemplate::New(RemoveSignalHandler)->GetFunction());
  proto_t->Set(String::NewSymbol("call"),
     FunctionTemplate::New(Call)->GetFunction());
  proto_t->SetNamedPropertyHandler(Getter);

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

struct WrappedMessage {
  static void Call(Handle<Function> callback, const EDBus_Message *msg);
};

Handle<Value> DProxy::Getter(Local<String> prop, const AccessorInfo& info)
{
   HandleScope scope;
   Local<Value> val = info.This()->GetRealNamedPropertyInPrototypeChain(prop);

   if (!val.IsEmpty())
     return Handle<Value>();

   Handle<Function> func = FunctionTemplate::New(Call)->GetFunction();
   func->Set(String::New("method"), prop);

   return scope.Close(func);
}

void WrappedMessage::Call(Handle<Function> callback, const EDBus_Message *msg)
{
   unsigned int argc = 0;
   Eina_Value *ev = NULL;
   Eina_Value_Struct st;
   const char *errname, *errmsg;

   if (edbus_message_error_get(msg, &errname, &errmsg))
     {
        fprintf(stderr, "Error: %s %s\n", errname, errmsg);
        return;
     }

   if (strlen(edbus_message_signature_get(msg)) > 0)
     {
        ev = edbus_message_to_eina_value(msg);
        eina_value_pget(ev, &st);
        argc = st.desc->member_count;
     }

   Handle<Value> argv[argc];

   for (unsigned int i = 0; i < argc; i++)
     {
        Eina_Value value;
        eina_value_struct_value_get(ev, st.desc->members[i].name, &value);
        argv[i] = einaValueToValue(&value);
     }

   callback->Call(Context::GetCurrent()->Global(), argc, argv);
}

struct WrappedSignalHandler : WrappedMessage {
public:
  WrappedSignalHandler(DProxy *proxy_, const char *signal_name_, Handle<Value> cb_, Handle<Value> data_)
    : cb(Persistent<Value>::New(cb_))
    , data(Persistent<Value>::New(data_))
    , sh(edbus_proxy_signal_handler_add(proxy_->GetProxy(),
           signal_name_,
           WrappedSignalHandler::Invoke,
           this)) {}

  ~WrappedSignalHandler();

private:
  Persistent<Value> cb;
  Persistent<Value> data;
  EDBus_Signal_Handler *sh;

  static void Invoke(void *data, const EDBus_Message *msg);
};

static Handle<Value> einaValueToValue(Eina_Value *value)
{
   Handle<Value> val = Undefined();

   const Eina_Value_Type *type = eina_value_type_get(value);

   if (EINA_VALUE_TYPE_INT == type)
     {
        int i;
        eina_value_get(value, &i);
        val = Number::New(i);
     }
   else if (EINA_VALUE_TYPE_STRING == type)
     {
        const char *s;
        eina_value_get(value, &s);
        val = String::New(s);
     }
   else if (EINA_VALUE_TYPE_UCHAR == type)
     {
        unsigned char c;
        eina_value_get(value, &c);
        val = Number::New(c);
     }
   else if (EINA_VALUE_TYPE_SHORT == type)
     {
        short i;
        eina_value_get(value, &i);
        val = Number::New(i);
     }
   else if (EINA_VALUE_TYPE_USHORT == type)
     {
        unsigned short i;
        eina_value_get(value, &i);
        val = Number::New(i);
     }
   else if (EINA_VALUE_TYPE_UINT == type)
     {
        unsigned int i;
        eina_value_get(value, &i);
        val = Number::New(i);
     }
   else if (EINA_VALUE_TYPE_INT64 == type)
     {
        int64_t i;
        eina_value_get(value, &i);
        val = Number::New(i);
     }
   else if (EINA_VALUE_TYPE_UINT64 == type)
     {
        uint64_t i;
        eina_value_get(value, &i);
        val = Number::New(i);
     }
   else if (EINA_VALUE_TYPE_DOUBLE == type)
     {
        double i;
        eina_value_get(value, &i);
        val = Number::New(i);
     }
   else if (EINA_VALUE_TYPE_ARRAY == type)
     {
        unsigned int len = eina_value_array_count(value);
        val = Array::New(len);
        for (unsigned int i = 0; i < len; i++)
          {
             Eina_Value v;
             eina_value_array_value_get(value, i, &v);
             val->ToObject()->Set(i, einaValueToValue(&v));
          }
     }
   else if (EINA_VALUE_TYPE_STRUCT == type)
     {
        Eina_Value_Struct st;
        eina_value_pget(value, &st);

        unsigned int len = st.desc->member_count;
        val = Array::New(len);

        for (unsigned int i = 0; i < len; i++)
         {
            Eina_Value v;
            eina_value_struct_value_get(value, st.desc->members[i].name, &v);
            val->ToObject()->Set(i, einaValueToValue(&v));
         }
     }
   else
     {
        WRN("Unexpected Type.");
     }

   return val;
}

void WrappedSignalHandler::Invoke(void *data, const EDBus_Message *msg)
{
   HandleScope scope;
   WrappedSignalHandler *self = static_cast<WrappedSignalHandler *>(data);
   Handle<Function> callback(Function::Cast(*self->cb));
   Call(callback, msg);
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
struct WrappedCallMessage : WrappedMessage {
public:
   WrappedCallMessage(Handle<Value> _callback)
      : cb(Persistent<Value>::New(_callback)) {}
   Persistent<Value> cb;
   static void Callback(void *data, const EDBus_Message *msg, EDBus_Pending *pending);
};

void WrappedCallMessage::Callback(void *data, const EDBus_Message *msg, EDBus_Pending *)
{
   HandleScope scope;
   WrappedCallMessage *self = static_cast<WrappedCallMessage *>(data);
   Handle<Function> callback(Function::Cast(*self->cb));
   Call(callback, msg);
}

static char getSigID(Handle<Value> val)
{
   char c = '?';
   Local<Value> sig = val->ToObject()->Get(String::NewSymbol("signature"));

   if (!sig->IsUndefined())
     {
        c = (*String::Utf8Value(sig))[0];
     }
   else
     {
        if (val->IsBoolean())
          c = 'b';
        else if (val->IsNumber())
          c = 'd';
        else if (val->IsString())
          c = 's';
        else if (val->IsArray())
          c = 'a';
        else
          WRN("Unexpected type.");
     }

   return c;
}

static Local<String> getSignature(Local<String> signature, Handle<Value> val)
{
   if (val->IsUndefined())
     return signature;

   const char sig[2] = { getSigID(val), '\0' };

   if ('r' == sig[0])
     {
        Local<Object> obj = val->ToObject();
        Local<Array> props = obj->GetOwnPropertyNames();
        unsigned int len = props->Length();

        signature = String::Concat(signature, String::New("("));
        for (unsigned int i = 0; i < len; i++)
          signature = getSignature(signature, obj->Get(props->Get(i)));
        signature = String::Concat(signature, String::New(")"));
     }
   else
     {
        signature = String::Concat(signature, String::New(sig));

        if (val->IsArray())
          {
             Local<Value> v = val->ToObject()->Get(0);
             signature = getSignature(signature, v);
          }
     }

   return signature;
}

static Local<String> getSignature(Handle<Value> val)
{
   return getSignature(String::New(""), val);
}

static void append(EDBus_Message_Iter *iter, Handle<Value> val)
{
   if (val->IsUndefined())
     return;

   const char sig = getSigID(val);

   switch (sig)
     {
      default:
         edbus_message_iter_basic_append(iter, sig, *String::Utf8Value(val));
         break;
      case 'b':
         edbus_message_iter_basic_append(iter, sig, val->BooleanValue());
         break;
      case 'n':
      case 'i':
      case 'x':
         edbus_message_iter_basic_append(iter, sig, val->Int32Value());
         break;
      case 'y':
      case 'q':
      case 'u':
      case 't':
         edbus_message_iter_basic_append(iter, sig, val->ToUint32()->Value());
         break;
      case 'd':
         edbus_message_iter_basic_append(iter, sig, val->NumberValue());
         break;
      case 'v':
      case 'a':
           {
              Local<Object> obj = val->ToObject();
              Local<Array> props = obj->GetOwnPropertyNames();
              EDBus_Message_Iter *sub_iter = edbus_message_iter_container_new
                 (iter, sig, &(*String::Utf8Value(getSignature(val)))[1]);

             for (unsigned int i = 0, len = props->Length(); i < len; i++)
               append(sub_iter, obj->Get(props->Get(i)));

             edbus_message_iter_container_close(iter, sub_iter);
             break;
           }
      case 'r':
           {
              EDBus_Message_Iter *sub_iter;
              Local<Object> obj = val->ToObject();
              Local<Array> props = obj->GetOwnPropertyNames();

              edbus_message_iter_arguments_set
                 (iter, *String::Utf8Value(getSignature(val)), &sub_iter);

              for (unsigned int i = 0, len = props->Length(); i < len; i++)
                append(sub_iter, obj->Get(props->Get(i)));

              edbus_message_iter_container_close(iter, sub_iter);
              break;
           }
     }
}

Handle<Value> DProxy::Call(const Arguments& args)
{
   HandleScope scope;

   DProxy *self = ObjectWrap::Unwrap<DProxy>(args.This());

   EDBus_Message *msg;
   EDBus_Message_Iter *iter;

   unsigned int args_cnt = 0;
   unsigned int last_arg = (args.Length() - 1);

   if (!args[last_arg]->IsFunction())
     return ThrowException
        (Exception::TypeError(String::New("Last argument must be a function")));

   Local<Value> method = args.Callee()->Get(String::New("method"));

   if (method->IsUndefined())
     method = args[args_cnt++];

   msg = edbus_proxy_method_call_new(self->proxy, *String::Utf8Value(method));
   iter = edbus_message_iter_get(msg);

   for (; args_cnt < last_arg; args_cnt++)
     append(iter, args[args_cnt]);

   edbus_proxy_send(self->proxy, msg, WrappedCallMessage::Callback,
                    new WrappedCallMessage(args[last_arg]), -1);

   edbus_message_unref(msg);

   return Undefined();
}

}
