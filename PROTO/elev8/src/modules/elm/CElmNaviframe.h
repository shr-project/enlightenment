#ifndef C_ELM_NAVIFRAME_H
#define C_ELM_NAVIFRAME_H

#include "elm.h"
#include "CElmLayout.h"
#include "CElmObject.h"

namespace elm {

class CElmNaviframe : public CElmLayout {
private:

   class Item {
   public:
      Persistent<Object> jsObject;
      Elm_Object_Item *object_item;

      Item(Elm_Object_Item *_object_item, Local<Object> obj, Handle<Value> parent)
        {
           HandleScope scope;
           static Persistent<ObjectTemplate> tmpl;

           if (tmpl.IsEmpty())
             {
                Local<FunctionTemplate> klass = FunctionTemplate::New();
                klass->SetClassName(String::NewSymbol("NaviframeItem"));

                tmpl = Persistent<ObjectTemplate>::New(klass->InstanceTemplate());

                tmpl->SetAccessor(String::NewSymbol("title"), GetTitle, SetTitle);
                tmpl->SetAccessor(String::NewSymbol("subtitle"), GetTitle, SetTitle);

                tmpl->SetAccessor(String::NewSymbol("icon"), GetContent, SetContent);
                tmpl->SetAccessor(String::NewSymbol("content"), GetContent, SetContent);
                tmpl->SetAccessor(String::NewSymbol("prev_btn"), GetContent, SetContent);
                tmpl->SetAccessor(String::NewSymbol("next_btn"), GetContent, SetContent);

                tmpl->SetAccessor(String::NewSymbol("style"), GetStyle, SetStyle);

                tmpl->SetAccessor(String::NewSymbol("title_visible"),
                                                    GetTitleVisible, SetTitleVisible);

                tmpl->Set(String::NewSymbol("promote"), FunctionTemplate::New(Promote));
             }

           object_item = _object_item;
           elm_object_item_data_set(object_item, this);
           elm_object_item_del_cb_set(object_item, Delete);

           jsObject = Persistent<Object>::New(tmpl->NewInstance());
           jsObject->SetHiddenValue(String::NewSymbol("parent"), parent);
           jsObject->SetHiddenValue(String::NewSymbol("item"), External::Wrap(this));

           Local<Array> props = obj->GetOwnPropertyNames();
           for (unsigned int i = 0; i < props->Length(); i++)
             {
                Local<Value> key = props->Get(i);
                jsObject->Set(key, obj->Get(key));
             }
        }

      ~Item()
        {
           Local<Function> callback
              (Function::Cast(*jsObject->Get(String::NewSymbol("on_delete"))));

           if (callback->IsFunction())
             callback->Call(jsObject, 0, NULL);

           jsObject->DeleteHiddenValue(String::NewSymbol("item"));
           jsObject.Dispose();
        }

      static Handle<Value> GetTitle(Local<String> name, const AccessorInfo &info)
        {
           String::Utf8Value part(name);

           return String::New
              (elm_object_item_part_text_get(Unwrap(info)->object_item,
                                             strcmp(*part, "title") ? *part : NULL));
        }

      static void SetTitle(Local<String> name, Local<Value> value, const AccessorInfo &info)
        {
           String::Utf8Value part(name);

           if (value->IsString())
             elm_object_item_part_text_set(Unwrap(info)->object_item,
                                           strcmp(*part, "title") ? *part : NULL,
                                           *String::Utf8Value(value));
        }

      static Handle<Value> GetContent(Local<String> name, const AccessorInfo &info)
        {
           String::Utf8Value part(name);

           Evas_Object *content = elm_object_item_part_content_get
              (Unwrap(info)->object_item, strcmp(*part, "content") ? *part : NULL);

           if (!content)
             return Undefined();

           CElmObject *obj = static_cast<CElmObject *>(evas_object_data_get(content, "this"));

           if (!obj)
             return Undefined();

           return obj->GetJSObject();
        }

      static void SetContent(Local<String> name, Local<Value> val, const AccessorInfo &info)
        {
           Item *item = Unwrap(info);
           String::Utf8Value part(name);
           Handle<Value> value = val;

           value = Realise(val, item->ToObject()->GetHiddenValue(String::NewSymbol("parent")));

           if (value->IsUndefined())
             elm_object_item_part_content_unset
                (item->object_item, strcmp(*part, "content") ? *part : NULL);
           else
             elm_object_item_part_content_set
                (item->object_item, strcmp(*part, "content") ? *part : NULL,
                 GetEvasObjectFromJavascript(value));
        }

      static Handle<Value> GetStyle(Local<String>, const AccessorInfo &info)
        {
           return String::New(elm_naviframe_item_style_get(Unwrap(info)->object_item));
        }

      static void SetStyle(Local<String>, Local<Value> value, const AccessorInfo &info)
        {
           if (value->IsString())
             elm_naviframe_item_style_set(Unwrap(info)->object_item, *String::Utf8Value(value));
        }

      static Handle<Value> GetTitleVisible(Local<String>, const AccessorInfo &info)
        {
           return Boolean::New
              (elm_naviframe_item_title_visible_get(Unwrap(info)->object_item));
        }

      static void SetTitleVisible(Local<String>, Local<Value> value, const AccessorInfo &info)
        {
           elm_naviframe_item_title_visible_set(Unwrap(info)->object_item,
                                                value->BooleanValue());
        }

      static Handle<Value> Promote(const Arguments& args)
        {
           elm_naviframe_item_promote(Unwrap(args.This())->object_item);
           return Undefined();
        }

      Handle<Object> ToObject()
        {
           return jsObject;
        }

      static void Delete(void *data, Evas_Object *, void *)
        {
           delete static_cast<Item *>(data);
        }

      static Item *Unwrap(const AccessorInfo& info)
        {
           return Unwrap(info.This());
        }

      static Item *Unwrap(Handle<Value> value)
        {
           if (!value->IsObject())
             return NULL;
           value = value->ToObject()->GetHiddenValue(String::NewSymbol("item"));
           if (value.IsEmpty())
             return NULL;
           return static_cast<Item *>(External::Unwrap(value));
        }
   };

   bool title_visible;
   static Persistent<FunctionTemplate> tmpl;

protected:
   CElmNaviframe(Local<Object> _jsObject, CElmObject *parent);

   static Handle<FunctionTemplate> GetTemplate();

   void title_visible_eval();

public:
   static void Initialize(Handle<Object> target);

   virtual Handle<Value> Pack(Handle<Value>, Handle<Value>);
   virtual Handle<Value> Unpack(Handle<Value>);

   Handle<Value> pop(const Arguments& args);
   Handle<Value> promote(const Arguments& args);
   Handle<Value> item_promote(const Arguments& args);
   Handle<Value> pop_to(const Arguments& args);

   void title_visible_set(Handle<Value> val);
   Handle<Value> title_visible_get() const;

   void event_enabled_set(Handle<Value> val);
   Handle<Value> event_enabled_get() const;

   void prev_btn_auto_pushed_set(Handle<Value> val);
   Handle<Value> prev_btn_auto_pushed_get() const;

   void content_preserve_on_pop_set(Handle<Value> val);
   Handle<Value> content_preserve_on_pop_get() const;

   Handle<Value> items_get() const;
   Handle<Value> top_item_get() const;
   Handle<Value> bottom_item_get() const;

   friend Handle<Value> CElmObject::New<CElmNaviframe>(const Arguments &args);
};

}

#endif
