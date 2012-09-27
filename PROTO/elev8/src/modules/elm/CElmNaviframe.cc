#include "elm.h"
#include "CElmNaviframe.h"

namespace elm {

using namespace v8;

GENERATE_PROPERTY_CALLBACKS(CElmNaviframe, title_visible);
GENERATE_PROPERTY_CALLBACKS(CElmNaviframe, event_enabled);
GENERATE_PROPERTY_CALLBACKS(CElmNaviframe, prev_btn_auto_pushed);
GENERATE_PROPERTY_CALLBACKS(CElmNaviframe, content_preserve_on_pop);
GENERATE_RO_PROPERTY_CALLBACKS(CElmNaviframe, top_item);
GENERATE_RO_PROPERTY_CALLBACKS(CElmNaviframe, bottom_item);
GENERATE_METHOD_CALLBACKS(CElmNaviframe, pop);
GENERATE_METHOD_CALLBACKS(CElmNaviframe, pop_to);

GENERATE_TEMPLATE_FULL(CElmLayout, CElmNaviframe,
                  PROPERTY(title_visible),
                  PROPERTY(event_enabled),
                  PROPERTY(prev_btn_auto_pushed),
                  PROPERTY(content_preserve_on_pop),
                  PROPERTY_RO(top_item),
                  PROPERTY_RO(bottom_item),
                  METHOD(pop),
                  METHOD(pop_to));


CElmNaviframe::CElmNaviframe(Local<Object> _jsObject, CElmObject *p)
   : CElmLayout(_jsObject, elm_naviframe_add(p->GetEvasObject()))
   , title_visible(true)
{
}

void CElmNaviframe::Initialize(Handle<Object> target)
{
   target->Set(String::NewSymbol("Naviframe"), GetTemplate()->GetFunction());
}

Handle<Value> CElmNaviframe::pop(const Arguments&)
{
   Evas_Object *content = elm_naviframe_item_pop(eo);
   if (!content)
     return Undefined();

   CElmObject *obj = static_cast<CElmObject *>(evas_object_data_get(content, "this"));
   if (!obj)
     return Undefined();

   return obj->GetJSObject();
}

Handle<Value> CElmNaviframe::Pack(Handle<Value> value, Handle<Value> replace)
{
   HandleScope scope;
   Local<Object> obj = value->ToObject();
   Local<String> str = String::NewSymbol("before");
   Local<Value> before = obj->Get(str);

   if (before->IsUndefined() && !replace->IsUndefined())
     before = replace->ToObject()->Get(str);
   else if (before->IsString() || before->IsNumber())
     before = GetJSObject()->Get(String::NewSymbol("elements"))->ToObject()->Get(before);

   obj->Set(str, before);

   Item *item = new Item(obj->ToObject(), GetJSObject());
   title_visible_eval();
   return scope.Close(item->ToObject());
}

Handle<Value> CElmNaviframe::Unpack(Handle<Value> value)
{
   elm_naviframe_item_pop(eo);
   return value;
}

Handle<Value> CElmNaviframe::pop_to(const Arguments& args)
{
   Item *item = Item::Unwrap(args[0]);
   elm_naviframe_item_pop_to(item->object_item);
   return Undefined();
}

void CElmNaviframe::title_visible_eval()
{
   if (Elm_Object_Item *top_item = elm_naviframe_top_item_get(eo))
      elm_naviframe_item_title_visible_set(top_item, title_visible);
}

void CElmNaviframe::title_visible_set(Handle<Value> val)
{
   title_visible = val->BooleanValue();
   title_visible_eval();
}

Handle<Value> CElmNaviframe::title_visible_get() const
{
   return Boolean::New(title_visible);
}

void CElmNaviframe::event_enabled_set(Handle<Value> val)
{
   if(val->IsBoolean())
     elm_naviframe_event_enabled_set(eo, val->BooleanValue());
}

Handle<Value> CElmNaviframe::event_enabled_get() const
{
   return Boolean::New(elm_naviframe_event_enabled_get(eo));
}

void CElmNaviframe::prev_btn_auto_pushed_set(Handle<Value> val)
{
   if(val->IsBoolean())
     elm_naviframe_prev_btn_auto_pushed_set(eo, val->BooleanValue());
}

Handle<Value> CElmNaviframe::prev_btn_auto_pushed_get() const
{
   return Boolean::New(elm_naviframe_prev_btn_auto_pushed_get(eo));
}

void CElmNaviframe::content_preserve_on_pop_set(Handle<Value> val)
{
   if(val->IsBoolean())
     elm_naviframe_content_preserve_on_pop_set(eo, val->BooleanValue());
}

Handle<Value> CElmNaviframe::content_preserve_on_pop_get() const
{
   return Boolean::New(elm_naviframe_content_preserve_on_pop_get(eo));
}

Handle<Value> CElmNaviframe::top_item_get() const
{
   Item *item = static_cast<Item *>
      (elm_object_item_data_get(elm_naviframe_top_item_get(eo)));
   return item->ToObject();
}

Handle<Value> CElmNaviframe::bottom_item_get() const
{
   Item *item = static_cast<Item *>
      (elm_object_item_data_get(elm_naviframe_bottom_item_get(eo)));
   return item->ToObject();
}

}
