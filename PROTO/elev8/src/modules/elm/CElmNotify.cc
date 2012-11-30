#include "elm.h"
#include "CElmNotify.h"

namespace elm {

using namespace v8;

GENERATE_PROPERTY_CALLBACKS(CElmNotify, content);
GENERATE_PROPERTY_CALLBACKS(CElmNotify, align);
GENERATE_PROPERTY_CALLBACKS(CElmNotify, timeout);
GENERATE_PROPERTY_CALLBACKS(CElmNotify, allow_events);
GENERATE_PROPERTY_CALLBACKS(CElmNotify, parent);


GENERATE_TEMPLATE_FULL(CElmContainer, CElmNotify,
                  PROPERTY(content),
                  PROPERTY(align),
                  PROPERTY(timeout),
                  PROPERTY(allow_events),
                  PROPERTY(parent));

CElmNotify::CElmNotify(Local<Object> _jsObject, CElmObject *p)
   : CElmContainer(_jsObject,
                   elm_notify_add(p->GetEvasObject()))
{
}

CElmNotify::~CElmNotify()
{
   cached.content.Dispose();
}

void CElmNotify::Initialize(Handle<Object> target)
{
   target->Set(String::NewSymbol("Notify"), GetTemplate()->GetFunction());
}

Handle<Value> CElmNotify::content_get() const
{
   return cached.content;
}

void CElmNotify::content_set(Handle<Value> val)
{
   cached.content.Dispose();
   cached.content = Persistent<Value>::New(Realise(val, jsObject));
   elm_object_content_set(eo, GetEvasObjectFromJavascript(cached.content));
}

Handle<Value> CElmNotify::align_get() const
{
   double x, y;
   Local<Object> obj;

   elm_notify_align_get(eo, &x, &y);
   obj->Set(String::NewSymbol("x"), Number::New(x));
   obj->Set(String::NewSymbol("y"), Number::New(y));

   return obj;
}

void CElmNotify::align_set(Handle<Value> val)
{
   if (!val->IsObject())
     return;

   Local<Object> obj = val->ToObject();
   double x = obj->Get(String::NewSymbol("x"))->NumberValue();
   double y = obj->Get(String::NewSymbol("y"))->NumberValue();
   elm_notify_align_set(eo, x, y);
}

Handle<Value> CElmNotify::timeout_get() const
{
   return Number::New(elm_notify_timeout_get(eo));
}

void CElmNotify::timeout_set(Handle<Value> val)
{
   if (val->IsNumber())
     elm_notify_timeout_set(eo, val->Int32Value());
}

Handle<Value> CElmNotify::allow_events_get() const
{
   return Boolean::New(elm_notify_allow_events_get(eo));
}

void CElmNotify::allow_events_set(Handle<Value> val)
{
   elm_notify_allow_events_set(eo, val->BooleanValue());
}

Handle<Value> CElmNotify::parent_get() const
{
   void *f_parent = evas_object_data_get(elm_notify_parent_get(eo), "this");
   return static_cast<CElmObject *>(f_parent)->GetJSObject();
}

void CElmNotify::parent_set(Handle<Value> val)
{
   if (val->IsObject())
     elm_notify_parent_set(eo, GetEvasObjectFromJavascript(val));
}

}
