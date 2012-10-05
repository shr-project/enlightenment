#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>

#include <Ecore.h>
#include <EDBus.h>
#include <Elocation.h>
#include <elocation_private.h>

static char *unique_name = NULL;
static EDBus_Connection *conn = NULL;;
static Elocation_Provider *master_provider;
static EDBus_Signal_Handler *cb_position_changed = NULL;
static EDBus_Signal_Handler *cb_address_changed = NULL;

EAPI int ELOCATION_EVENT_IN;
EAPI int ELOCATION_EVENT_OUT;
EAPI int ELOCATION_EVENT_STATUS;
EAPI int ELOCATION_EVENT_POSITION;
EAPI int ELOCATION_EVENT_ADDRESS;

static void
provider_info_cb(void *data , const EDBus_Message *reply, EDBus_Pending *pending)
{
   char *name = NULL, *desc = NULL;
   const char *signature;

   signature = edbus_message_signature_get(reply);
   if (strcmp(signature, "ss")) return;

   if (!edbus_message_arguments_get(reply, "ss", &name, &desc)) return;

   printf("Provider name: %s\n description: %s\n", name, desc);
}

static void
unmarshall_address(Elocation_Address *address, const EDBus_Message *reply)
{
   int32_t level, timestamp;
   EDBus_Message_Iter *iter, *sub, *dict, *entry;
   double horizontal;
   double vertical;
   const char *key, *signature;
   char *value;

   signature = edbus_message_signature_get(reply);
   if (strcmp(signature,"ia{ss}(idd)")) return;

   iter = edbus_message_iter_get(reply);

   edbus_message_iter_get_and_next(iter, 'i', &timestamp);
   address->timestamp = timestamp;

   edbus_message_iter_arguments_get(iter, "a{ss}", &dict);

   while (edbus_message_iter_get_and_next(dict, 'e', &entry))
    {
       edbus_message_iter_arguments_get(entry, "ss", &key, &value);

       if (!strcmp(key, "country"))
        {
            address->country = strdup(value);
         }
       else if (!strcmp(key, "countrycode"))
         {
            address->countrycode = strdup(value);
         }
       else if (!strcmp(key, "locality"))
         {
            address->locality = strdup(value);
         }
       else if (!strcmp(key, "postalcode"))
         {
            address->postalcode = strdup(value);
         }
       else if (!strcmp(key, "region"))
         {
            address->region = strdup(value);
         }
       else if (!strcmp(key, "timezone"))
         {
            address->timezone = strdup(value);
         }
    }

   edbus_message_iter_get_and_next(iter, 'r', &sub );
   edbus_message_iter_arguments_get(sub, "idd", &level, &horizontal, &vertical);
   address->accur->level = level;
   address->accur->horizontal = horizontal;
   address->accur->vertical = vertical;
}

static void
address_cb(void *data , const EDBus_Message *reply, EDBus_Pending *pending)
{
   Elocation_Address *address;

   address = malloc(sizeof(Elocation_Address));
   address->accur = malloc(sizeof(Elocation_Accuracy));

   unmarshall_address(address, reply);
   ecore_event_add(ELOCATION_EVENT_ADDRESS, address, NULL, NULL);
}
static void
address_signal_cb(void *data , const EDBus_Message *reply)
{
   Elocation_Address *address;

   address = malloc(sizeof(Elocation_Address));
   address->accur = malloc(sizeof(Elocation_Accuracy));

   unmarshall_address(address, reply);
   ecore_event_add(ELOCATION_EVENT_ADDRESS, address, NULL, NULL);
}

static void
unmarshall_position(Elocation_Position *position, const EDBus_Message *reply)
{
   GeocluePositionFields fields;
   int32_t level, timestamp;
   double horizontal = 0.0;
   double vertical = 0.0;
   double latitude = 0.0;
   double longitude = 0.0;
   double altitude = 0.0;
   EDBus_Message_Iter *iter, *sub;
   const char *err, *errmsg;
   const char *signature;

   if (edbus_message_error_get(reply, &err, &errmsg))
     {
        fprintf(stderr, "Error: %s %s\n", err, errmsg);
        return;
     }

   signature = edbus_message_signature_get(reply);
   if (strcmp(signature, "iiddd(idd)"))
     {
        fprintf(stderr, "Error: position callback message did not match signature\n");
        return;
     }

   iter = edbus_message_iter_get(reply);

   // Possible to use a single edbus_message_iter_arguments_get(sub, "iiddd(idd)" ... here?

   edbus_message_iter_get_and_next(iter, 'i', &fields);
   position->fields = fields;

   edbus_message_iter_get_and_next(iter, 'i', &timestamp);
   position->timestamp = timestamp;

   edbus_message_iter_get_and_next(iter, 'd', &latitude);
   position->latitude = latitude;

   edbus_message_iter_get_and_next(iter, 'd', &longitude);
   position->longitude = longitude;

   edbus_message_iter_get_and_next(iter, 'd', &altitude);
   position->altitude = altitude;

   edbus_message_iter_get_and_next(iter, 'r', &sub );
   edbus_message_iter_arguments_get(sub, "idd", &level, &horizontal, &vertical);
   position->accur->level = level;
   position->accur->horizontal = horizontal;
   position->accur->vertical = vertical;
}

static void
position_cb(void *data , const EDBus_Message *reply, EDBus_Pending *pending)
{
   Elocation_Position *position;

   position = malloc(sizeof(Elocation_Position));
   position->accur = malloc(sizeof(Elocation_Accuracy));

   unmarshall_position(position, reply);
   ecore_event_add(ELOCATION_EVENT_POSITION, position, NULL, NULL);
}

static void
position_signal_cb(void *data , const EDBus_Message *reply)
{
   Elocation_Position *position;

   position = malloc(sizeof(Elocation_Position));
   position->accur = malloc(sizeof(Elocation_Accuracy));

   unmarshall_position(position, reply);
   ecore_event_add(ELOCATION_EVENT_POSITION, position, NULL, NULL);
}
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

   master_provider->status = status;
   ecore_event_add(ELOCATION_EVENT_STATUS, &status, NULL, NULL);
}

static void
status_signal_cb(void *data , const EDBus_Message *reply)
{
   int32_t status;

   if (edbus_message_signature_get(reply) != "i") return;

   if (!edbus_message_arguments_get(reply,"i",  &status)) return;

   master_provider->status = status;
   ecore_event_add(ELOCATION_EVENT_STATUS, &status, NULL, NULL);
}

EAPI Eina_Bool
elocation_address_get(Elocation_Address *address)
{
   EDBus_Pending *pending;
   EDBus_Object *obj_ubuntu;
   EDBus_Proxy *manager_address;

   obj_ubuntu = edbus_object_get(conn, UBUNTU_DBUS_NAME, UBUNTU_OBJECT_PATH);
   if (!obj_ubuntu)
     {
        fprintf(stderr, "Error: could not get object\n");
        return EXIT_FAILURE;
     }

   manager_address = edbus_proxy_get(obj_ubuntu, GEOCLUE_ADDRESS_IFACE);
   if (!manager_address)
     {
        fprintf(stderr, "Error: could not get proxy\n");
        return EXIT_FAILURE;
     }

   pending = edbus_proxy_call(manager_address, "GetAddress", address_cb, NULL, -1, "");
   if (!pending)
     {
        fprintf(stderr, "Error: could not call\n");
        return EXIT_FAILURE;
     }
}

EAPI Eina_Bool
elocation_position_get(Elocation_Position *position)
{
   EDBus_Pending *pending;
   EDBus_Object *obj_ubuntu;
   EDBus_Proxy *manager_position;

   obj_ubuntu = edbus_object_get(conn, UBUNTU_DBUS_NAME, UBUNTU_OBJECT_PATH);
   if (!obj_ubuntu)
     {
        fprintf(stderr, "Error: could not get object\n");
        return EXIT_FAILURE;
     }

   manager_position = edbus_proxy_get(obj_ubuntu, GEOCLUE_POSITION_IFACE);
   if (!manager_position)
     {
        fprintf(stderr, "Error: could not get proxy\n");
        return EXIT_FAILURE;
     }

   pending = edbus_proxy_call(manager_position, "GetPosition", position_cb, NULL, -1, "");
   if (!pending)
     {
        fprintf(stderr, "Error: could not call\n");
        return EXIT_FAILURE;
     }
}

EAPI Eina_Bool
elocation_status_get(int *status)
{
   EDBus_Pending *pending;
   EDBus_Object *obj_geoclue;
   EDBus_Proxy *manager_geoclue;

   obj_geoclue = edbus_object_get(conn, GEOCLUE_DBUS_NAME, GEOCLUE_OBJECT_PATH);
   if (!obj_geoclue)
     {
        fprintf(stderr, "Error: could not get object\n");
        return EXIT_FAILURE;
     }

   manager_geoclue = edbus_proxy_get(obj_geoclue, GEOCLUE_IFACE);
   if (!manager_geoclue)
     {
        fprintf(stderr, "Error: could not get proxy\n");
        return EXIT_FAILURE;
     }

   pending = edbus_proxy_call(manager_geoclue, "GetStatus", status_cb, NULL, -1, "");
   if (!pending)
     {
        fprintf(stderr, "Error: could not call\n");
        return EXIT_FAILURE;
     }
}

EAPI Eina_Bool
elocation_init()
{
   EDBus_Message *msg;
   EDBus_Object *obj_ubuntu, *obj_geoclue;
   EDBus_Proxy *manager_geoclue, *manager_ubuntu, *manager_client, *manager_address, *manager_position;
   EDBus_Pending *pending, *pending2;

   conn = edbus_connection_get(EDBUS_CONNECTION_TYPE_SESSION);
   if (!conn)
     {
      printf("Error: could not connect to session bus.\n");
      return EXIT_FAILURE;
     }

   if (ELOCATION_EVENT_IN == 0)
      ELOCATION_EVENT_IN = ecore_event_type_new();

   if (ELOCATION_EVENT_OUT == 0)
      ELOCATION_EVENT_OUT = ecore_event_type_new();

   if (ELOCATION_EVENT_STATUS == 0)
      ELOCATION_EVENT_STATUS = ecore_event_type_new();

   if (ELOCATION_EVENT_POSITION == 0)
      ELOCATION_EVENT_POSITION = ecore_event_type_new();

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

   manager_ubuntu = edbus_proxy_get(obj_ubuntu, GEOCLUE_IFACE);
   if (!manager_ubuntu)
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

   manager_address = edbus_proxy_get(obj_ubuntu, GEOCLUE_ADDRESS_IFACE);
   if (!manager_address)
     {
        fprintf(stderr, "Error: could not get proxy\n");
        return EXIT_FAILURE;
     }

   manager_position = edbus_proxy_get(obj_ubuntu, GEOCLUE_POSITION_IFACE);
   if (!manager_position)
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

   pending2 = edbus_proxy_call(manager_ubuntu, "GetProviderInfo", provider_info_cb, NULL, -1, "");
   if (!pending2)
     {
        fprintf(stderr, "Error: could not call\n");
        return EXIT_FAILURE;
     }

   cb_address_changed = edbus_proxy_signal_handler_add(manager_address, "GetAddress", address_signal_cb, NULL);
   cb_position_changed = edbus_proxy_signal_handler_add(manager_position, "GetPosition", position_signal_cb, NULL);

   edbus_name_owner_changed_callback_add(conn, GEOCLUE_DBUS_NAME, _name_owner_changed,
                                         NULL, EINA_TRUE);

   ecore_event_handler_add(ELOCATION_EVENT_IN, geoclue_start, NULL);
   ecore_event_handler_add(ELOCATION_EVENT_OUT, geoclue_stop, NULL);

   edbus_signal_handler_add(conn, UBUNTU_DBUS_NAME, UBUNTU_OBJECT_PATH, GEOCLUE_POSITION_IFACE, "GetStatus",
         status_signal_cb, NULL);
}

EAPI void
elocation_shutdown()
{
   edbus_name_owner_changed_callback_del(conn, GEOCLUE_DBUS_NAME, _name_owner_changed, NULL);
   edbus_signal_handler_unref(cb_address_changed);
   edbus_signal_handler_unref(cb_position_changed);
   edbus_connection_unref(conn);
}
