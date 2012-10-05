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
   int *status = event;

   printf("Status changed to: %i\n", *status);
   return ECORE_CALLBACK_DONE;
}

static Eina_Bool
address_changed(void *data, int ev_type, void *event)
{
   Elocation_Address *address;

   address = event;
   printf("Address update with data from timestamp: %i\n", address->timestamp);
   printf("Country: %s\n", address->country);
   printf("Countrycode: %s\n", address->countrycode);
   printf("Locality: %s\n", address->locality);
   printf("Postalcode: %s\n", address->postalcode);
   printf("Region: %s\n", address->region);
   printf("Timezone: %s\n", address->timezone);
   printf("Accuracy level: %i\n", address->accur->level);
   printf("Accuracy horizemtal: %f\n", address->accur->horizontal);
   printf("Accuracy vertical: %f\n", address->accur->vertical);

   return ECORE_CALLBACK_DONE;
}

static Eina_Bool
position_changed(void *data, int ev_type, void *event)
{
   Elocation_Position *position;

   position = event;
   printf("GeoClue position reply with data from timestamp %i\n", position->timestamp);
   if (position->fields & GEOCLUE_POSITION_FIELDS_LATITUDE)
      printf("Latitude:\t %f (valid)\n", position->latitude);
   else
      printf("Latitude:\tinvalid.\n");

   if (position->fields & GEOCLUE_POSITION_FIELDS_LONGITUDE)
      printf("Longitude:\t %f (valid)\n", position->longitude);
   else
      printf("Longitude:\tinvalid.\n");

   if (position->fields & GEOCLUE_POSITION_FIELDS_ALTITUDE)
      printf("Altitude:\t %f (valid)\n", position->altitude);
   else
      printf("Altitude:\tinvalid.\n");

   printf("Accuracy level: %i\n", position->accur->level);
   printf("Accuracy horizemtal: %f\n", position->accur->horizontal);
   printf("Accuracy vertical: %f\n", position->accur->vertical);
   return ECORE_CALLBACK_DONE;
}

int
main()
{
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
