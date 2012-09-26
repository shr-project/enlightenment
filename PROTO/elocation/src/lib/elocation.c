#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>

#include <Ecore.h>
#include <E_DBus.h>
#include <Elocation.h>

EAPI int elocation_init(void)
{
   // listen for NameOwnerChanges to send out ecore events when geoclue appears or disappears
   if (E_LOCATION_EVENT_IN == 0)
      E_LOCATION_EVENT_IN = ecore_event_type_new();

   if (E_LOCATION_EVENT_OUT == 0)
      E_LOCATION_EVENT_OUT = ecore_event_type_new();

}

