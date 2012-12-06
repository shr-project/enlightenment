#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>

#include <Ecore.h>
#include <Elocation.h>

static Eina_Bool
status_changed(void *data, int ev_type, void *event)
{
   int *status = event;

   printf("Status changed to: %i\n", *status);
   return ECORE_CALLBACK_DONE;
}

static Eina_Bool
rgeocode_arrived(void *data, int ev_type, void *event)
{
   Elocation_Address *address;

   address = event;
   printf("Geocode reply:\n");
   printf("Country: %s\n", address->country);
   printf("Countrycode: %s\n", address->countrycode);
   printf("Locality: %s\n", address->locality);
   printf("Postalcode: %s\n", address->postalcode);
   printf("Region: %s\n", address->region);
   printf("Timezone: %s\n", address->timezone);
   printf("Accuracy level: %i\n", address->accur->level);
   printf("Accuracy horizontal: %f\n", address->accur->horizontal);
   printf("Accuracy vertical: %f\n", address->accur->vertical);

   return ECORE_CALLBACK_DONE;
}

static Eina_Bool
geocode_arrived(void *data, int ev_type, void *event)
{
   Elocation_Position *position;

   position = event;
   printf("Reverse geocode reply:\n");
   printf("Latitude:\t %f\n", position->latitude);
   printf("Longitude:\t %f\n", position->longitude);
   printf("Altitude:\t %f\n", position->altitude);
   printf("Accuracy level: %i\n", position->accur->level);
   printf("Accuracy horizontal: %f\n", position->accur->horizontal);
   printf("Accuracy vertical: %f\n", position->accur->vertical);
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
   printf("Accuracy horizontal: %f\n", address->accur->horizontal);
   printf("Accuracy vertical: %f\n", address->accur->vertical);

   return ECORE_CALLBACK_DONE;
}

static Eina_Bool
position_changed(void *data, int ev_type, void *event)
{
   Elocation_Position *position;

   position = event;
   printf("GeoClue position reply with data from timestamp %i\n", position->timestamp);
   printf("Latitude:\t %f\n", position->latitude);
   printf("Longitude:\t %f\n", position->longitude);
   printf("Altitude:\t %f\n", position->altitude);
   printf("Accuracy level: %i\n", position->accur->level);
   printf("Accuracy horizontal: %f\n", position->accur->horizontal);
   printf("Accuracy vertical: %f\n", position->accur->vertical);
   return ECORE_CALLBACK_DONE;
}

int
main(void)
{
   Elocation_Address *address, *addr_geocode;
   Elocation_Position *position, *pos_geocode;
   int status;

   ecore_init();
   edbus_init();
   elocation_init();

   address = elocation_address_new();
   position = elocation_position_new();

   /* Register callback so we get updates later on */
   ecore_event_handler_add(ELOCATION_EVENT_STATUS, status_changed, NULL);
   ecore_event_handler_add(ELOCATION_EVENT_POSITION, position_changed, NULL);
   ecore_event_handler_add(ELOCATION_EVENT_ADDRESS, address_changed, NULL);
   ecore_event_handler_add(ELOCATION_EVENT_GEOCODE, geocode_arrived, NULL);
   ecore_event_handler_add(ELOCATION_EVENT_REVERSEGEOCODE, rgeocode_arrived, NULL);

   elocation_status_get(&status);
   elocation_position_get(position);
   elocation_address_get(address);

   addr_geocode = elocation_address_new();
   pos_geocode = elocation_position_new();

   elocation_freeform_address_to_position("London", pos_geocode);

   pos_geocode->latitude = 51.7522;
   pos_geocode->longitude = -1.25596;
   pos_geocode->accur->level = 3;
   elocation_position_to_address(pos_geocode, addr_geocode);

   addr_geocode->locality = "Cambridge";
   addr_geocode->countrycode = "UK";
   elocation_address_to_position(addr_geocode, pos_geocode);

   ecore_main_loop_begin();

   elocation_address_free(addr_geocode);
   elocation_position_free(pos_geocode);
   elocation_address_free(address);
   elocation_position_free(position);
   elocation_shutdown();
   edbus_shutdown();
   ecore_shutdown();
   return 0;
}
