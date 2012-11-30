#include "dbus-connection.h"
#include "dbus-pending.h"
#include "dbus-object.h"

namespace dbus {

DPending::DPending(EDBus_Pending *pending_)
  : pending(pending_)
{
}

DPending::~DPending()
{
  edbus_pending_cancel(pending);
}

void DPending::Init(Handle<Object>)
{
  HandleScope scope;

  Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
  tpl->SetClassName(String::NewSymbol("DPending"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
}

Handle<Value> DPending::New(const Arguments& args)
{
  HandleScope scope;

  if (!args[0]->IsObject())
    THROW_EXCEPTION("Expecting pending object");

  EDBus_Pending *wrapped = static_cast<EDBus_Pending *>(External::Unwrap(args[0]->ToObject()));
  DPending *pending = new DPending(wrapped);
  pending->Wrap(args.This());

  return args.This();
}

}
