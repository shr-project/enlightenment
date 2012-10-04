#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>

#include <Ecore.h>
#include <Elocation.h>
#include <elocation_private.h>

/* TODO:
   o Create a client object to operate on
   o Ask master for providers
   o Set requirements
   o Give out ecore events on signals
   o Start (how to stop?) service to get position updates
   o How to deal with AddReference and RemoveReference?
   o Implement MasterClient interface
   o Reply on address is inconsistent. Either all NULL or all empty
*/

static Eina_Bool
status_changed(void *data, int ev_type, void *event)
{
   unsigned int status = (unsigned int)&event;
   printf("Status changed to: %i\n", status);
   return ECORE_CALLBACK_DONE;
}

static Eina_Bool
position_changed(void *data, int ev_type, void *event)
{
   return ECORE_CALLBACK_DONE;
}

static Eina_Bool
address_changed(void *data, int ev_type, void *event)
{
   return ECORE_CALLBACK_DONE;
}

int
main()
{
   Elocation_Accuracy *accuracy;
   Elocation_Address *address;
   Elocation_Position *position;
   int status;

   ecore_init();
   edbus_init();
   elocation_init();

   /* Register callback so we get updates later on */
   ecore_event_handler_add(ELOCATION_EVENT_STATUS, status_changed, NULL);
   ecore_event_handler_add(ELOCATION_EVENT_POSITION, position_changed, NULL);
   ecore_event_handler_add(ELOCATION_EVENT_ADDRESS, address_changed, NULL);

   elocation_status_get(&status);
   elocation_position_get(position);
   elocation_address_get(address);

   ecore_main_loop_begin();

   elocation_shutdown();
   edbus_shutdown();
   ecore_shutdown();
   return 0;
}
