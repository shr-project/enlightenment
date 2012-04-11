#ifndef C_ELM_BACKGROUND_H
#define C_ELM_BACKGROUND_H

#include "elm.h"
#include "CElmObject.h"

namespace elm {

using namespace v8;

class CElmBackground : public CElmObject {
private:
   static Persistent<FunctionTemplate> tmpl;

protected:
   CElmBackground(Local<Object> _jsObject, CElmObject *parent);
   static Handle<FunctionTemplate> GetTemplate();
   static void Delete(Persistent<Value>, void *parameter);

public:
   static void Initialize(Handle<Object> target);

   void image_set(Handle<Value> val);
   Handle<Value> image_get(void) const;

   Handle<Value> red_get() const;
   void red_set(Handle<Value> val);

   Handle<Value> green_get() const;
   void green_set(Handle<Value> val);

   Handle<Value> blue_get() const;
   void blue_set(Handle<Value> val);

   friend Handle<Value> CElmObject::New<CElmBackground>(const Arguments& args);
};

}

#endif // C_ELM_BACKGROUND_H
