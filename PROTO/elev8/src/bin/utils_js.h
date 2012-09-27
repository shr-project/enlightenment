#ifndef __UTILS_JS_H__
#define __UTILS_JS_H__

#include <v8.h>

using namespace v8;

namespace utils {

void RegisterModule(Handle<ObjectTemplate>);

}
#endif

