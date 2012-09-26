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
   o Attach to signals and give out ecore events
   o Start (how to stop?) service to get position updates
   o Look for nameOwner changed to issue IN and OUT for geoclue
*/

static E_DBus_Signal_Handler *cb_position_changed = NULL;

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

//   ecore_main_loop_quit();
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
   DBusMessage *msg, *msg2, *msg3;
   DBusMessageIter iter, sub;
   int ret = 0;
   struct gc_accuracy *accur;

   e_dbus_init();
   elocation_init();

   conn = e_dbus_bus_get(DBUS_BUS_SESSION);
   if (!conn)
     {
      printf("Error: could not connect to session bus.\n");
      ret = 1;
     }

   msg3 = dbus_message_new_method_call(GEOCLUE_DBUS_NAME, GEOCLUE_OBJECT_PATH, GEOCLUE_DBUS_NAME, "Create");
   e_dbus_message_send(conn, msg3, create_cb, -1, NULL);
   dbus_message_unref(msg3);

#if 0
   dbus_message_append_args(msg, DBUS_TYPE_INT32, &field, DBUS_TYPE_INT32, &timestamp,
                                DBUS_TYPE_DOUBLE, &latitude, DBUS_TYPE_DOUBLE, &longitude,
                                DBUS_TYPE_DOUBLE, &altitude, DBUS_TYPE_INVALID);

   dbus_message_iter_init_append(msg, &iter);
   if (dbus_message_iter_open_container(&iter, DBUS_TYPE_STRUCT, NULL, &sub))
     {
      dbus_message_iter_append_basic(&sub, DBUS_TYPE_INT32, &level);
      dbus_message_iter_append_basic(&sub, DBUS_TYPE_DOUBLE, &horizontal);
      dbus_message_iter_append_basic(&sub, DBUS_TYPE_DOUBLE, &vertical);
     }
   else
      printf("dbus_message_iter_open_container() failed\n");

   dbus_message_iter_close_container(&iter, &sub);
#endif
   msg = dbus_message_new_method_call(UBUNTU_DBUS_NAME, UBUNTU_OBJECT_PATH, GEOCLUE_POSITION_IFACE, "GetPosition");
   e_dbus_message_send(conn, msg, position_cb, -1, NULL);
   dbus_message_unref(msg);

   cb_position_changed = e_dbus_signal_handler_add(conn, UBUNTU_DBUS_NAME, UBUNTU_OBJECT_PATH, GEOCLUE_POSITION_IFACE, "GetPosition",
         position_signal_cb, NULL);

   msg2 = dbus_message_new_method_call(UBUNTU_DBUS_NAME, UBUNTU_OBJECT_PATH, GEOCLUE_IFACE, "GetStatus");
   e_dbus_message_send(conn, msg2, status_cb, -1, NULL);
   dbus_message_unref(msg2);

   ecore_main_loop_begin();

   e_dbus_shutdown();
   return ret;
}
