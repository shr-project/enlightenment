#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>

#include <Ecore.h>
#include <E_DBus.h>
#include <Elocation.h>

/* TODO:
   o Create a client object to operate on
   o Ask master for providers
   o Set requirements
   o Give out ecore events on signals
   o Start (how to stop?) service to get position updates
   o How to deal with AddReference and RemoveReference?
   o Implement MasterClient interface
*/

typedef struct _gc_address
{
   char *country;
   char *countrycode;
   char *locality;
   char *postalcode;
   char *region;
   char *timezone;
} gc_address;

static E_DBus_Signal_Handler *cb_position_changed = NULL;

static Eina_Bool
geoclue_start(void *data, int ev_type, void *event)
{
   printf("GeoClue start event\n");
   return ECORE_CALLBACK_DONE;
}

static Eina_Bool
geoclue_stop(void *data, int ev_type, void *event)
{
   printf("GeoClue stop event\n");
   return ECORE_CALLBACK_DONE;
}

void
unmarshal_address(DBusMessageIter *iter)
{
   DBusMessageIter arr;
   const char *key;
   dbus_message_iter_recurse(iter, &arr);
   gc_address address;

   if (dbus_message_iter_get_arg_type(&arr) == DBUS_TYPE_INVALID)
     return;

   do
     {
        DBusMessageIter dict;
        dbus_message_iter_recurse(&arr, &dict);
        do
          {
             char *value;

             dbus_message_iter_get_basic(&dict, &key);
             dbus_message_iter_next(&dict);

             if (!strcmp(key, "country"))
              {
                  dbus_message_iter_get_basic(&dict, &value);
                  address.country = value;
                  printf("Key: %s, value: %s\n", key, value);
               }
             else if (!strcmp(key, "countrycode"))
               {
                  dbus_message_iter_get_basic(&dict, &value);
                  address.countrycode = value;
                  printf("Key: %s, value: %s\n", key, value);
               }
             else if (!strcmp(key, "locality"))
               {
                  dbus_message_iter_get_basic(&dict, &value);
                  address.locality = value;
                  printf("Key: %s, value: %s\n", key, value);
               }
             else if (!strcmp(key, "postalcode"))
               {
                  dbus_message_iter_get_basic(&dict, &value);
                  address.postalcode = value;
                  printf("Key: %s, value: %s\n", key, value);
               }
             else if (!strcmp(key, "region"))
               {
                  dbus_message_iter_get_basic(&dict, &value);
                  address.region = value;
                  printf("Key: %s, value: %s\n", key, value);
               }
             else if (!strcmp(key, "timezone"))
               {
                  dbus_message_iter_get_basic(&dict, &value);
                  address.timezone = value;
                  printf("Key: %s, value: %s\n", key, value);
               }
          }
        while (dbus_message_iter_next(&dict));
     }
   while (dbus_message_iter_next(&arr));
}

void
create_cb(void *data , DBusMessage *reply, DBusError *error)
{
   const char *object_path;
   DBusMessageIter iter;

   if (!dbus_message_has_signature(reply, "o")) return;

   dbus_message_iter_init(reply, &iter);
   dbus_message_iter_get_basic(&iter, &object_path);

   printf("Object path for client: %s\n", object_path);
}

void
status_cb(void *data , DBusMessage *reply, DBusError *error)
{
   dbus_int32_t status;
   DBusMessageIter iter;

   if (!dbus_message_has_signature(reply, "i")) return;

   dbus_message_iter_init(reply, &iter);
   dbus_message_iter_get_basic(&iter, &status);

   printf("Status: %i\n", status);
}

void
status_signal_cb(void *data , DBusMessage *reply)
{
   status_cb(data, reply, NULL);
}

void
provider_info_cb(void *data , DBusMessage *reply, DBusError *error)
{
   dbus_int32_t status;
   DBusMessageIter iter;
   char *name = NULL, *desc = NULL;

   if (!dbus_message_has_signature(reply, "ss")) return;

   dbus_message_get_args(reply, error,
                        DBUS_TYPE_STRING, &name,
                        DBUS_TYPE_STRING, &desc,
                        DBUS_TYPE_INVALID);

   printf("Provider name: %s\n description: %s\n", name, desc);
}

void
address_cb(void *data , DBusMessage *reply, DBusError *error)
{
   dbus_int32_t level, timestamp;
   DBusMessageIter iter, sub;
   double horizontal;
   double vertical;
   gc_accuracy accur;

   if (dbus_error_is_set(error))
     {
      printf("Error: %s - %s\n", error->name, error->message);
      return;
     }

   if (!dbus_message_has_signature(reply, "ia{ss}(idd)")) return;

   dbus_message_iter_init(reply, &iter);

   dbus_message_iter_get_basic(&iter, &timestamp);
   dbus_message_iter_next(&iter);

   unmarshal_address(&iter);
   dbus_message_iter_next(&iter);

   dbus_message_iter_recurse(&iter, &sub);
   dbus_message_iter_get_basic(&sub, &level);
   accur.level = level;
   dbus_message_iter_next(&sub);

   dbus_message_iter_get_basic(&sub, &horizontal);
   accur.horizontal = horizontal;
   dbus_message_iter_next(&sub);

   dbus_message_iter_get_basic(&sub, &vertical);
   accur.vertical = vertical;
   dbus_message_iter_next(&sub);
}

void
address_signal_cb(void *data , DBusMessage *reply)
{
   address_cb(data, reply, NULL);
}

void
position_cb(void *data , DBusMessage *reply, DBusError *error)
{
   GeocluePositionFields fields;
   dbus_int32_t level, timestamp;
   double horizontal = 0.0;
   double vertical = 0.0;
   double latitude = 0.0;
   double longitude = 0.0;
   double altitude = 0.0;
   DBusMessageIter iter, sub;
   gc_accuracy accur;

   if (dbus_error_is_set(error))
     {
      printf("Error: %s - %s\n", error->name, error->message);
      return;
     }

   if (!dbus_message_has_signature(reply, "iiddd(idd)")) return;

   dbus_message_iter_init(reply, &iter);

   dbus_message_iter_get_basic(&iter, &fields);
   dbus_message_iter_next(&iter);

   dbus_message_iter_get_basic(&iter, &timestamp);
   dbus_message_iter_next(&iter);

   dbus_message_iter_get_basic(&iter, &latitude);
   dbus_message_iter_next(&iter);

   dbus_message_iter_get_basic(&iter, &longitude);
   dbus_message_iter_next(&iter);

   dbus_message_iter_get_basic(&iter, &altitude);
   dbus_message_iter_next(&iter);

   dbus_message_iter_recurse(&iter, &sub);
   dbus_message_iter_get_basic(&sub, &level);
   accur.level = level;
   dbus_message_iter_next(&sub);

   dbus_message_iter_get_basic(&sub, &horizontal);
   accur.horizontal = horizontal;
   dbus_message_iter_next(&sub);

   dbus_message_iter_get_basic(&sub, &vertical);
   accur.vertical = vertical;
   dbus_message_iter_next(&sub);

   printf("DBus GeoClue reply with data from timestamp %i\n", timestamp);

   if (fields & GEOCLUE_POSITION_FIELDS_LATITUDE)
      printf("Latitude:\t %f (valid)\n", latitude);
   else
      printf("Latitude:\tinvalid.\n");

   if (fields & GEOCLUE_POSITION_FIELDS_LONGITUDE)
      printf("Longitude:\t %f (valid)\n", longitude);
   else
      printf("Longitude:\tinvalid.\n");

   if (fields & GEOCLUE_POSITION_FIELDS_ALTITUDE)
      printf("Altitude:\t %f (valid)\n", altitude);
   else
      printf("Altitude:\tinvalid.\n");

   printf("Accuracy: level %i, horizontal %f and vertical %f\n", level, horizontal, vertical);
}

void
position_signal_cb(void *data , DBusMessage *reply)
{
   position_cb(data, reply, NULL);
}

int
main()
{
   E_DBus_Connection *conn;
   DBusMessage *msg;
   DBusMessageIter iter, sub;
   int ret = 0;
   struct gc_accuracy *accur;

   e_dbus_init();

   conn = e_dbus_bus_get(DBUS_BUS_SESSION);
   if (!conn)
     {
      printf("Error: could not connect to session bus.\n");
      ret = 1;
     }

   elocation_init(conn);

   ecore_event_handler_add(E_LOCATION_EVENT_IN, geoclue_start, NULL);
   ecore_event_handler_add(E_LOCATION_EVENT_OUT, geoclue_stop, NULL);

   msg = dbus_message_new_method_call(GEOCLUE_DBUS_NAME, GEOCLUE_OBJECT_PATH, GEOCLUE_DBUS_NAME, "Create");
   e_dbus_message_send(conn, msg, create_cb, -1, NULL);
   dbus_message_unref(msg);
   msg = NULL;

   msg = dbus_message_new_method_call(UBUNTU_DBUS_NAME, UBUNTU_OBJECT_PATH, GEOCLUE_IFACE, "GetStatus");
   e_dbus_message_send(conn, msg, status_cb, -1, NULL);
   dbus_message_unref(msg);
   msg = NULL;

   cb_position_changed = e_dbus_signal_handler_add(conn, UBUNTU_DBUS_NAME, UBUNTU_OBJECT_PATH, GEOCLUE_POSITION_IFACE, "GetStatus",
         status_signal_cb, NULL);

   msg = dbus_message_new_method_call(UBUNTU_DBUS_NAME, UBUNTU_OBJECT_PATH, GEOCLUE_IFACE, "GetProviderInfo");
   e_dbus_message_send(conn, msg, provider_info_cb, -1, NULL);
   dbus_message_unref(msg);
   msg = NULL;

   // SetOptions

   msg = dbus_message_new_method_call(UBUNTU_DBUS_NAME, UBUNTU_OBJECT_PATH, GEOCLUE_ADDRESS_IFACE, "GetAddress");
   e_dbus_message_send(conn, msg, address_cb, -1, NULL);
   dbus_message_unref(msg);
   msg = NULL;

   cb_position_changed = e_dbus_signal_handler_add(conn, UBUNTU_DBUS_NAME, UBUNTU_OBJECT_PATH, GEOCLUE_ADDRESS_IFACE, "GetAddress",
         address_signal_cb, NULL);

   msg = dbus_message_new_method_call(UBUNTU_DBUS_NAME, UBUNTU_OBJECT_PATH, GEOCLUE_POSITION_IFACE, "GetPosition");
   e_dbus_message_send(conn, msg, position_cb, -1, NULL);
   dbus_message_unref(msg);
   msg = NULL;

   cb_position_changed = e_dbus_signal_handler_add(conn, UBUNTU_DBUS_NAME, UBUNTU_OBJECT_PATH, GEOCLUE_POSITION_IFACE, "GetPosition",
         position_signal_cb, NULL);

   ecore_main_loop_begin();

   elocation_shutdown(conn);
   e_dbus_shutdown();
   return ret;
}
