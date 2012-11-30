#include "dbus-connection.h"
#include "dbus-message.h"

namespace dbus {

Persistent<FunctionTemplate> DMessage::base_template;

DMessage::~DMessage()
{
  edbus_message_unref(msg);
}

void DMessage::Init(Handle<Object> target)
{
  HandleScope scope;

  DMessage::base_template = Persistent<FunctionTemplate>::New(FunctionTemplate::New(New));
  DMessage::base_template->SetClassName(String::NewSymbol("DMessage"));

  Local<ObjectTemplate> ot = DMessage::base_template->InstanceTemplate();
  ot->SetInternalFieldCount(1);
  ot->SetAccessor(String::NewSymbol("path"), GetPath);
  ot->SetAccessor(String::NewSymbol("interface"), GetInterface);
  ot->SetAccessor(String::NewSymbol("member"), GetMember);
  ot->SetAccessor(String::NewSymbol("sender"), GetSender);
  ot->SetAccessor(String::NewSymbol("signature"), GetSignature);
  ot->SetAccessor(String::NewSymbol("error"), GetError);

  Local<ObjectTemplate> pt = DMessage::base_template->PrototypeTemplate();
  pt->Set(String::NewSymbol("reply"), FunctionTemplate::New(NewMethodReturn)->GetFunction());
  pt->Set(String::NewSymbol("error"), FunctionTemplate::New(NewError)->GetFunction());

  Local<FunctionTemplate> tpl = FunctionTemplate::New();
  tpl->Inherit(DMessage::base_template);
  tpl->Set(String::NewSymbol("type"), String::NewSymbol("method-call"));
  target->Set(String::NewSymbol("MethodCall"), tpl->GetFunction());

  tpl = FunctionTemplate::New();
  tpl->Inherit(DMessage::base_template);
  tpl->Set(String::NewSymbol("type"), String::NewSymbol("signal"));
  target->Set(String::NewSymbol("Signal"), tpl->GetFunction());
}

Handle<Value> DMessage::New(const Arguments& args)
{
  HandleScope scope;
  
  Local<Value> type = args.This()->Get(String::NewSymbol("type"));
  if (!type.IsEmpty()) {
    String::Utf8Value type_str(type->ToString());

    if (!strcmp(*type_str, "method-call"))
      return DMessage::NewCall(args);
    if (!strcmp(*type_str, "signal"))
      return DMessage::NewSignal(args);
  }

  THROW_EXCEPTION("Invalid messsage type");
}

Handle<Value> DMessage::NewCall(const Arguments& args)
{
  printf("Creating new method call\n");

  if (!args[0]->IsString())
    THROW_EXCEPTION("Expecting destination as string");
  if (!args[1]->IsString())
    THROW_EXCEPTION("Expecting path as string");
  if (!args[2]->IsString())
    THROW_EXCEPTION("Expecting interface as string");
  if (!args[3]->IsString())
    THROW_EXCEPTION("Expecting method as string");

  EDBus_Message *msg = edbus_message_method_call_new(
     *String::Utf8Value(args[0]),
     *String::Utf8Value(args[1]),
     *String::Utf8Value(args[2]),
     *String::Utf8Value(args[3]));
  if (!msg)
    THROW_EXCEPTION("Could not build method call message");

  DMessage *msg_obj = new DMessage(msg);
  msg_obj->Wrap(args.This());

  return args.This();
}

Handle<Value> DMessage::NewSignal(const Arguments& args)
{
  printf("Creating new signal\n");

  if (!args[0]->IsString())
    THROW_EXCEPTION("Expecting path as string");
  if (!args[1]->IsString())
    THROW_EXCEPTION("Expecting interface as string");
  if (!args[2]->IsString())
    THROW_EXCEPTION("Expecting name as string");

  EDBus_Message *msg = edbus_message_signal_new(
     *String::Utf8Value(args[0]),
     *String::Utf8Value(args[1]),
     *String::Utf8Value(args[2]));
  if (!msg)
    THROW_EXCEPTION("Could not build signal message");

  DMessage *msg_obj = new DMessage(msg);
  msg_obj->Wrap(args.This());

  return args.This();
}

Handle<Value> DMessage::NewMethodReturn(const Arguments& args)
{
  printf("Creating new method return\n");

  HandleScope scope;
  DMessage *self = ObjectWrap::Unwrap<DMessage>(args.This());
  EDBus_Message *msg = edbus_message_method_return_new(self->msg);
  if (!msg)
    THROW_EXCEPTION("Could not build reply message");

  DMessage *msg_obj = new DMessage(msg);
  Local<Object> new_obj = Object::New();  
  msg_obj->Wrap(new_obj);

  return scope.Close(new_obj);
}

Handle<Value> DMessage::NewError(const Arguments& args)
{
  printf("Creating new error\n");

  HandleScope scope;
  DMessage *reply = ObjectWrap::Unwrap<DMessage>(args.This());

  if (!args[0]->IsString())
    THROW_EXCEPTION("Expecting error_name as string");
  if (!args[1]->IsString())
    THROW_EXCEPTION("Expecting error_msg as string");

  EDBus_Message *msg = edbus_message_error_new(reply->GetMessage(),
     *String::Utf8Value(args[0]),
     *String::Utf8Value(args[1]));
  if (!msg)
    THROW_EXCEPTION("Could not build error message");

  DMessage *msg_obj = new DMessage(msg);
  Local<Object> new_obj = Object::New();
  msg_obj->Wrap(new_obj);

  return scope.Close(new_obj);
}

Handle<Value> DMessage::GetPath(Local<String>, const AccessorInfo& info)
{
  HandleScope scope;
  DMessage *self = ObjectWrap::Unwrap<DMessage>(info.This());

  return scope.Close(String::New(edbus_message_path_get(self->msg)));
}

Handle<Value> DMessage::GetInterface(Local<String>, const AccessorInfo& info)
{
  HandleScope scope;
  DMessage *self = ObjectWrap::Unwrap<DMessage>(info.This());

  return scope.Close(String::New(edbus_message_interface_get(self->msg)));
}

Handle<Value> DMessage::GetMember(Local<String>, const AccessorInfo& info)
{
  HandleScope scope;
  DMessage *self = ObjectWrap::Unwrap<DMessage>(info.This());

  return scope.Close(String::New(edbus_message_member_get(self->msg)));
}

Handle<Value> DMessage::GetSender(Local<String>, const AccessorInfo& info)
{
  HandleScope scope;
  DMessage *self = ObjectWrap::Unwrap<DMessage>(info.This());

  return scope.Close(String::New(edbus_message_sender_get(self->msg)));
}

Handle<Value> DMessage::GetSignature(Local<String>, const AccessorInfo& info)
{
  HandleScope scope;
  DMessage *self = ObjectWrap::Unwrap<DMessage>(info.This());

  return scope.Close(String::New(edbus_message_signature_get(self->msg)));
}

Handle<Value> DMessage::GetError(Local<String>, const AccessorInfo& info)
{
  HandleScope scope;
  DMessage *self = ObjectWrap::Unwrap<DMessage>(info.This());
  const char *name, *text;
  Local<Object> obj = Object::New();
  
  if (!edbus_message_error_get(self->msg, &name, &text)) {
    name = "success";
    text = "Not an error message";
  }

  obj->Set(String::NewSymbol("name"), String::NewSymbol(name));
  obj->Set(String::NewSymbol("text"), String::New(text));

  return scope.Close(obj);
}

bool DMessage::IsMessage(Handle<Value> v)
{
  if (v.IsEmpty())
    return false;
  if (!v->IsObject())
    return false;
  return DMessage::base_template->HasInstance(v->ToObject());
}

}
