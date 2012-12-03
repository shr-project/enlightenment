#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/* STD */
#include <cassert>

#include "ecorexx/Event.h"

namespace Ecorexx {

Event::Event(int type, sigc::slot<bool, int, void*> handler) :
  mHandler(handler),
  mEventHandler(ecore_event_handler_add(type, Event::eventHandler, this))
{
}

Event::~Event()
{
  assert(ecore_event_handler_del(mEventHandler));
}

Eina_Bool Event::eventHandler(void *data, int type, void *event)
{
  Event *event_obj = static_cast<Event*>(data);
  assert(event_obj);
  
  return event_obj->mHandler(type, event);
}

} // end namespace Ecorexx
