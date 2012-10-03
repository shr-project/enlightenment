#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>

#include <Ecore.h>
#include <EDBus.h>
#include <Elocation.h>
#include <elocation_private.h>

static char *unique_name = NULL;
static Elocation_Provider master_provider;

static Eina_Bool
geoclue_start(void *data, int ev_type, void *event)
{
   printf("GeoClue start event at %s\n", unique_name);
   return ECORE_CALLBACK_DONE;
}

static Eina_Bool
geoclue_stop(void *data, int ev_type, void *event)
{
   printf("GeoClue stop event\n");
   return ECORE_CALLBACK_DONE;
}

static void
create_cb(void *data , const EDBus_Message *reply, EDBus_Pending *pending)
{
   const char *object_path;

   if (edbus_message_signature_get(reply) != "o") return;
   if (!edbus_message_arguments_get(reply, "o", &object_path)) return;

   printf("Object path for client: %s\n", object_path);
}

static void
_name_owner_changed(void *data , const char *bus, const char *old, const char *new)
{
   if (strcmp(bus, GEOCLUE_DBUS_NAME) != 0)
      return;

   if (old[0] == '\0' && new[0] != '\0')
     {
        ecore_event_add(ELOCATION_EVENT_IN, NULL, NULL, NULL);
        unique_name = strdup(new);
     }
   else if (old[0] != '\0' && new[0] == '\0')
     {
        if (strcmp(unique_name, old) != 0)
           printf("%s was not the known name %s, ignored.\n", old, unique_name);
        else
           ecore_event_add(ELOCATION_EVENT_OUT, NULL, NULL, NULL);
     }
   else
     {
        printf("unknow change from %s to %s\n", old, new);
     }
}

static void
status_cb(void *data , const EDBus_Message *reply, EDBus_Pending *pendding)
{
   int32_t status;

   if (edbus_message_signature_get(reply) != "i") return;

   if (!edbus_message_arguments_get(reply,"i",  &status)) return;

   master_provider.status = status;

   printf("Status: %i\n", master_provider.status);
}

static void
status_signal_cb(void *data , const EDBus_Message *reply)
{
   int32_t status;

   if (edbus_message_signature_get(reply) != "i") return;

   if (!edbus_message_arguments_get(reply,"i",  &status)) return;

   ecore_event_add(ELOCATION_EVENT_STATUS, &status, NULL, NULL);
   master_provider.status = status;
}

Eina_Bool
elocation_init(EDBus_Connection *conn)
{
   EDBus_Message *msg;
   EDBus_Object *obj_ubuntu, *obj_geoclue;
   EDBus_Proxy *manager, *manager_client;
   EDBus_Pending *pending, *pending2;;

   if (ELOCATION_EVENT_IN == 0)
      ELOCATION_EVENT_IN = ecore_event_type_new();

   if (ELOCATION_EVENT_OUT == 0)
      ELOCATION_EVENT_OUT = ecore_event_type_new();

   if (ELOCATION_EVENT_STATUS == 0)
      ELOCATION_EVENT_STATUS = ecore_event_type_new();

   if (ELOCATION_EVENT_STATUS == 0)
      ELOCATION_EVENT_STATUS = ecore_event_type_new();

   if (ELOCATION_EVENT_ADDRESS == 0)
      ELOCATION_EVENT_ADDRESS = ecore_event_type_new();

   obj_ubuntu = edbus_object_get(conn, UBUNTU_DBUS_NAME, UBUNTU_OBJECT_PATH);
   if (!obj_ubuntu)
     {
        fprintf(stderr, "Error: could not get object\n");
        return EXIT_FAILURE;
     }

   obj_geoclue = edbus_object_get(conn, GEOCLUE_DBUS_NAME, GEOCLUE_OBJECT_PATH);
   if (!obj_geoclue)
     {
        fprintf(stderr, "Error: could not get object\n");
        return EXIT_FAILURE;
     }

   manager = edbus_proxy_get(obj_geoclue, GEOCLUE_IFACE);
   if (!manager)
     {
        fprintf(stderr, "Error: could not get proxy\n");
        return EXIT_FAILURE;
     }

   manager_client = edbus_proxy_get(obj_geoclue, GEOCLUE_DBUS_NAME);
   if (!manager_client)
     {
        fprintf(stderr, "Error: could not get proxy\n");
        return EXIT_FAILURE;
     }

   pending = edbus_proxy_call(manager_client, "Create", create_cb, NULL, -1, "");
   if (!pending)
     {
        fprintf(stderr, "Error: could not call\n");
        return EXIT_FAILURE;
     }

   pending2 = edbus_proxy_call(manager, "GetStatus", status_cb, NULL, -1, "");
   if (!pending2)
     {
        fprintf(stderr, "Error: could not call\n");
        return EXIT_FAILURE;
     }

   edbus_name_owner_changed_callback_add(conn, GEOCLUE_DBUS_NAME, _name_owner_changed,
                                         NULL, EINA_TRUE);

   ecore_event_handler_add(ELOCATION_EVENT_IN, geoclue_start, NULL);
   ecore_event_handler_add(ELOCATION_EVENT_OUT, geoclue_stop, NULL);

   edbus_signal_handler_add(conn, UBUNTU_DBUS_NAME, UBUNTU_OBJECT_PATH, GEOCLUE_POSITION_IFACE, "GetStatus",
         status_signal_cb, NULL);
}

void
elocation_shutdown(EDBus_Connection *conn)
{
   edbus_name_owner_changed_callback_del(conn, GEOCLUE_DBUS_NAME, _name_owner_changed, NULL);
}
