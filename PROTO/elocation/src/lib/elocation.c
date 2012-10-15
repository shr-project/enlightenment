#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>

#include <Eina.h>
#include <Ecore.h>
#include <EDBus.h>
#include <Elocation.h>
#include <elocation_private.h>

static char *unique_name = NULL;
static EDBus_Connection *conn = NULL;
static Elocation_Provider *provider = NULL;
static EDBus_Signal_Handler *cb_position_changed = NULL;
static EDBus_Signal_Handler *cb_address_changed = NULL;
static EDBus_Signal_Handler *cb_status_changed = NULL;
static EDBus_Signal_Handler *cb_meta_address_provider_changed = NULL;
static EDBus_Signal_Handler *cb_meta_position_provider_changed = NULL;
static EDBus_Object *obj_ubuntu = NULL;
static EDBus_Object *obj_meta = NULL;
static EDBus_Proxy *ubuntu_geoclue = NULL;
static EDBus_Proxy *ubuntu_address = NULL;
static EDBus_Proxy *ubuntu_position = NULL;
static EDBus_Proxy *meta_geoclue = NULL;
static EDBus_Proxy *meta_address = NULL;
static EDBus_Proxy *meta_position = NULL;
static EDBus_Proxy *meta_masterclient = NULL;
static Elocation_Address *address = NULL;
static Elocation_Position *position = NULL;

int _elocation_log_dom = -1;

EAPI int ELOCATION_EVENT_IN;
EAPI int ELOCATION_EVENT_OUT;
EAPI int ELOCATION_EVENT_STATUS;
EAPI int ELOCATION_EVENT_POSITION;
EAPI int ELOCATION_EVENT_ADDRESS;

static void
_dummy_free(void *user_data, void *func_data)
{
   /* Don't free the event data after dispatching the event. We keep track of
    * it on our own */
}

static void
provider_info_cb(void *data, const EDBus_Message *reply, EDBus_Pending *pending)
{
   char *name = NULL, *desc = NULL;
   const char *signature;

   signature = edbus_message_signature_get(reply);
   if (strcmp(signature, "ss")) return;

   if (!edbus_message_arguments_get(reply, "ss", &name, &desc)) return;

   DBG("Provider name: %s, %s", provider->name, provider->description);
}

static void
meta_provider_info_cb(void *data, const EDBus_Message *reply, EDBus_Pending *pending)
{
   char *name = NULL, *desc = NULL, *service = NULL, *path = NULL;
   const char *signature;

   signature = edbus_message_signature_get(reply);
   if (strcmp(signature, "ssss")) return;

   if (!edbus_message_arguments_get(reply, "ssss", &name, &desc, &service, &path)) return;

   DBG("Meta provider name: %s, %s, %s, %s", provider->name, provider->description, service, path);
}

static void
meta_provider_info_signal_cb(void *data, const EDBus_Message *reply)
{
   char *name = NULL, *desc = NULL, *service = NULL, *path = NULL;
   const char *signature;

   signature = edbus_message_signature_get(reply);
   if (strcmp(signature, "ssss")) return;

   if (!edbus_message_arguments_get(reply, "ssss", &name, &desc, &service, &path)) return;
   provider->name = strdup(name);
   provider->description = strdup(desc);

   DBG("Meta provider name: %s, %s, %s, %s", provider->name, provider->description, service, path);
}

static void
unmarshall_address(const EDBus_Message *reply)
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

   address->country = NULL;
   address->countrycode = NULL;
   address->locality = NULL;
   address->postalcode = NULL;
   address->region = NULL;
   address->timezone = NULL;

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

   edbus_message_iter_get_and_next(iter, 'r', &sub);
   edbus_message_iter_arguments_get(sub, "idd", &level, &horizontal, &vertical);
   address->accur->level = level;
   address->accur->horizontal = horizontal;
   address->accur->vertical = vertical;
}

static void
address_cb(void *data, const EDBus_Message *reply, EDBus_Pending *pending)
{
   unmarshall_address(reply);
   ecore_event_add(ELOCATION_EVENT_ADDRESS, address, _dummy_free, NULL);
}
static void
address_signal_cb(void *data, const EDBus_Message *reply)
{
   unmarshall_address(reply);
   ecore_event_add(ELOCATION_EVENT_ADDRESS, address, _dummy_free, NULL);
}

static void
unmarshall_position(const EDBus_Message *reply)
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
        ERR("Error: %s %s", err, errmsg);
        return;
     }

   signature = edbus_message_signature_get(reply);
   if (strcmp(signature, "iiddd(idd)"))
     {
        ERR("Error: position callback message did not match signature");
        return;
     }

   iter = edbus_message_iter_get(reply);

   // Possible to use a single edbus_message_iter_arguments_get(sub, "iiddd(idd)" ... here?

   edbus_message_iter_get_and_next(iter, 'i', &fields);
   edbus_message_iter_get_and_next(iter, 'i', &timestamp);
   edbus_message_iter_get_and_next(iter, 'd', &latitude);
   edbus_message_iter_get_and_next(iter, 'd', &longitude);
   edbus_message_iter_get_and_next(iter, 'd', &altitude);
   edbus_message_iter_get_and_next(iter, 'r', &sub );
   edbus_message_iter_arguments_get(sub, "idd", &level, &horizontal, &vertical);

   position->timestamp = timestamp;

   /* GeoClue uses soem flags to mark position fields as valid. We set invalid
    * fields to 0.0 */
   if (fields & GEOCLUE_POSITION_FIELDS_LATITUDE)
      position->latitude = latitude;
   else
      position->latitude = 0.0;

   if (fields & GEOCLUE_POSITION_FIELDS_LONGITUDE)
      position->longitude = longitude;
   else
      position->longitude = 0.0;

   if (fields & GEOCLUE_POSITION_FIELDS_ALTITUDE)
      position->altitude = altitude;
   else
      position->altitude = 0.0;

   position->accur->level = level;
   position->accur->horizontal = horizontal;
   position->accur->vertical = vertical;
}

static void
position_cb(void *data, const EDBus_Message *reply, EDBus_Pending *pending)
{
   unmarshall_position(reply);
   ecore_event_add(ELOCATION_EVENT_POSITION, position, _dummy_free, NULL);
}

static void
position_signal_cb(void *data, const EDBus_Message *reply)
{
   unmarshall_position(reply);
   ecore_event_add(ELOCATION_EVENT_POSITION, position, _dummy_free, NULL);
}
static Eina_Bool
geoclue_start(void *data, int ev_type, void *event)
{
   DBG("GeoClue start event at %s", unique_name);
   return ECORE_CALLBACK_DONE;
}

static Eina_Bool
geoclue_stop(void *data, int ev_type, void *event)
{
   DBG("GeoClue stop event");
   return ECORE_CALLBACK_DONE;
}

static void
_reference_add_cb(void *data, const EDBus_Message *reply, EDBus_Pending *pending)
{
   DBG("Reference added");
}

static void
_reference_del_cb(void *data, const EDBus_Message *reply, EDBus_Pending *pending)
{
   DBG("Reference removed");
}

static void
status_cb(void *data, const EDBus_Message *reply, EDBus_Pending *pending)
{
   int *status;
   const char *signature;

   /* We need this to be malloced to be passed to ecore_event_add. Or provide a dummy free callback. */
   status = malloc(sizeof(*status));

   signature = edbus_message_signature_get(reply);
   if (strcmp(signature, "i"))
     {
        ERR("Error: status callback message did not match signature");
        return;
     }

   if (!edbus_message_arguments_get(reply,"i",  status)) return;

   provider->status = *status;
   ecore_event_add(ELOCATION_EVENT_STATUS, status, NULL, NULL);
}

static void
status_signal_cb(void *data, const EDBus_Message *reply)
{
   int *status;
   const char *signature;

   /* We need this to be malloced to be passed to ecore_event_add. Or provide a dummy free callback. */
   status = malloc(sizeof(*status));

   signature = edbus_message_signature_get(reply);
   if (strcmp(signature, "i"))
     {
        ERR("Error: status signal callback message did not match signature");
        return;
     }

   if (!edbus_message_arguments_get(reply,"i",  status)) return;

   provider->status = *status;
   ecore_event_add(ELOCATION_EVENT_STATUS, status, NULL, NULL);
}

static void
_dummy_cb(void *data, const EDBus_Message *msg, EDBus_Pending *pending)
{
}

static void
create_cb(void *data, const EDBus_Message *reply, EDBus_Pending *pending)
{
   const char *object_path;
   const char *signature;
   EDBus_Pending *pending1, *pending2;
   Eina_Bool updates;
   int accur_level, time, resources;

   signature = edbus_message_signature_get(reply);
   if (strcmp(signature, "o"))
     {
        ERR("Error: create callback message did not match signature");
        return;
     }

   if (!edbus_message_arguments_get(reply, "o", &object_path)) return;

   DBG("Object path for client: %s", object_path);

   /* With the created object path we now have a meta provider we can operate on.
    * Geoclue handles the selection of the best provider internally for the meta
    * provider */
   obj_meta = edbus_object_get(conn, GEOCLUE_DBUS_NAME, object_path);
   if (!obj_meta)
     {
        ERR("Error: could not get object");
        return;
     }

   meta_geoclue = edbus_proxy_get(obj_meta, GEOCLUE_GEOCLUE_IFACE);
   if (!meta_geoclue)
     {
        ERR("Error: could not get proxy");
        return;
     }

   meta_address = edbus_proxy_get(obj_meta, GEOCLUE_ADDRESS_IFACE);
   if (!meta_address)
     {
        ERR("Error: could not get proxy");
        return;
     }

   meta_position = edbus_proxy_get(obj_meta, GEOCLUE_POSITION_IFACE);
   if (!meta_position)
     {
        ERR("Error: could not get proxy");
        return;
     }

   meta_masterclient = edbus_proxy_get(obj_meta, GEOCLUE_MASTERCLIENT_IFACE);
   if (!meta_masterclient)
     {
        ERR("Error: could not get proxy");
        return;
     }

   /* Send Geoclue a set of requirements we have for the provider and start the address and position
    * meta provider afterwards. After this we should be ready for operation. */
   updates = EINA_FALSE; /* Especially the web providers do not offer updates */
   accur_level = ELOCATION_ACCURACY_LEVEL_COUNTRY;
   time = 0; /* Still need to figure out what this is used for */
   resources = ELOCATION_RESOURCE_ALL;

   cb_meta_address_provider_changed = edbus_proxy_signal_handler_add(meta_masterclient, "AddressProviderChanged", meta_provider_info_signal_cb, NULL);
   cb_meta_position_provider_changed = edbus_proxy_signal_handler_add(meta_masterclient, "PositionProviderChanged", meta_provider_info_signal_cb, NULL);

   edbus_proxy_call(meta_masterclient, "SetRequirements", _dummy_cb, NULL, -1, "iibi", accur_level, time, updates, resources);
   edbus_proxy_call(meta_masterclient, "AddressStart", _dummy_cb, NULL, -1, "");
   edbus_proxy_call(meta_masterclient, "PositionStart", _dummy_cb, NULL, -1, "");

   pending1 = edbus_proxy_call(meta_geoclue, "AddReference", _reference_add_cb, NULL, -1, "");
   if (!pending1)
     {
        ERR("Error: could not call");
        return;
     }

   pending = edbus_proxy_call(meta_address, "GetAddress", address_cb, NULL, -1, "");
   if (!pending)
     {
        ERR("Error: could not call");
        return;
     }

   pending = edbus_proxy_call(meta_position, "GetPosition", position_cb, NULL, -1, "");
   if (!pending)
     {
        ERR("Error: could not call");
        return;
     }

   pending2 = edbus_proxy_call(meta_geoclue, "GetProviderInfo", provider_info_cb, NULL, -1, "");
   if (!pending2)
     {
        ERR("Error: could not call");
        return;
     }

   pending2 = edbus_proxy_call(meta_masterclient, "GetAddressProvider", meta_provider_info_cb, NULL, -1, "");
   if (!pending2)
     {
        ERR("Error: could not call");
        return;
     }

   pending2 = edbus_proxy_call(meta_masterclient, "GetPositionProvider", meta_provider_info_cb, NULL, -1, "");
   if (!pending2)
     {
        ERR("Error: could not call");
        return;
     }

   cb_address_changed = edbus_proxy_signal_handler_add(meta_address, "AddressChanged", address_signal_cb, NULL);
   cb_position_changed = edbus_proxy_signal_handler_add(meta_position, "PositionChanged", position_signal_cb, NULL);
   cb_status_changed = edbus_proxy_signal_handler_add(meta_geoclue, "StatusChanged", status_signal_cb, NULL);
}

static void
_name_owner_changed(void *data, const char *bus, const char *old, const char *new)
{
   if (old[0] == '\0' && new[0] != '\0')
     {
        ecore_event_add(ELOCATION_EVENT_IN, NULL, NULL, NULL);
        unique_name = strdup(new);
     }
   else if (old[0] != '\0' && new[0] == '\0')
     {
        if (strcmp(unique_name, old) != 0)
           WARN("%s was not the known name %s, ignored.", old, unique_name);
        else
           ecore_event_add(ELOCATION_EVENT_OUT, NULL, NULL, NULL);
     }
   else
     {
        DBG("unknow change from %s to %s", old, new);
     }
}

EAPI Eina_Bool
elocation_address_get(Elocation_Address *address_shadow)
{
   if (!address) return EINA_FALSE;

   address_shadow = address;
   return EINA_TRUE;
}

EAPI Eina_Bool
elocation_position_get(Elocation_Position *position_shadow)
{
   if (!position) return EINA_FALSE;

   position_shadow = position;
   return EINA_TRUE;
}

EAPI Eina_Bool
elocation_status_get(int *status)
{
   EDBus_Pending *pending;

   pending = edbus_proxy_call(ubuntu_geoclue, "GetStatus", status_cb, NULL, -1, "");
   if (!pending)
     {
        ERR("Error: could not call");
        return EXIT_FAILURE;
     }
}

EAPI Elocation_Position *
elocation_position_new()
{
   /* Malloc the global struct we operate on here in this lib. This shadows the
    * updated data we are giving to the application */
   position = calloc(1, sizeof(Elocation_Position));
   if (!position) return NULL;

   position->accur = calloc(1, sizeof(Elocation_Accuracy));
   if (!position->accur) return NULL;

   return position;
}

EAPI Elocation_Address *
elocation_address_new()
{
   /* Malloc the global struct we operate on here in this lib. This shadows the
    * updated data we are giving to the application */
   address = calloc(1, sizeof(Elocation_Address));
   if (!address) return NULL;

   address->accur = calloc(1, sizeof(Elocation_Accuracy));
   if (!address->accur) return NULL;

   return address;
}

EAPI void
elocation_position_free(Elocation_Position *position_shadow)
{
   if (position != position_shadow)
     {
        ERR("Corrupted position object");
        return;
     }

   free(position->accur);
   free(position);
}

EAPI void
elocation_address_free(Elocation_Address *address_shadow)
{
   if (address != address_shadow)
     {
        ERR("Corrupted address object");
        return;
     }

   free(address->accur);
   free(address);
}

EAPI Eina_Bool
elocation_init()
{
   EDBus_Message *msg;
   EDBus_Object *obj_master = NULL;
   EDBus_Proxy *manager_master = NULL;
   EDBus_Pending *pending, *pending2, *pending3;

   if (!eina_init()) return EINA_FALSE;
   if (!ecore_init()) return EINA_FALSE;
   if (!edbus_init()) return EINA_FALSE;

   _elocation_log_dom = eina_log_domain_register("elocation", EINA_COLOR_BLUE);
   if (_elocation_log_dom < 0)
     {
        EINA_LOG_ERR("Could not register 'elocation' log domain.");
     }

   conn = edbus_connection_get(EDBUS_CONNECTION_TYPE_SESSION);
   if (!conn)
     {
      ERR("Error: could not connect to session bus.");
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
        ERR("Error: could not get object");
        return EXIT_FAILURE;
     }

   obj_master= edbus_object_get(conn, GEOCLUE_DBUS_NAME, GEOCLUE_OBJECT_PATH);
   if (!obj_master)
     {
        ERR("Error: could not get object");
        return EXIT_FAILURE;
     }

   ubuntu_geoclue = edbus_proxy_get(obj_ubuntu, GEOCLUE_GEOCLUE_IFACE);
   if (!ubuntu_geoclue)
     {
        ERR("Error: could not get proxy");
        return EXIT_FAILURE;
     }

   manager_master = edbus_proxy_get(obj_master, GEOCLUE_MASTER_IFACE);
   if (!manager_master)
     {
        ERR("Error: could not get proxy");
        return EXIT_FAILURE;
     }

   ubuntu_address = edbus_proxy_get(obj_ubuntu, GEOCLUE_ADDRESS_IFACE);
   if (!ubuntu_address)
     {
        ERR("Error: could not get proxy");
        return EXIT_FAILURE;
     }

   ubuntu_position = edbus_proxy_get(obj_ubuntu, GEOCLUE_POSITION_IFACE);
   if (!ubuntu_position)
     {
        ERR("Error: could not get proxy");
        return EXIT_FAILURE;
     }

   pending = edbus_proxy_call(manager_master, "Create", create_cb, NULL, -1, "");
   if (!pending)
     {
        ERR("Error: could not call");
        return EXIT_FAILURE;
     }

   pending = edbus_proxy_call(ubuntu_address, "GetAddress", address_cb, NULL, -1, "");
   if (!pending)
     {
        ERR("Error: could not call");
        return EXIT_FAILURE;
     }

   pending = edbus_proxy_call(ubuntu_position, "GetPosition", position_cb, NULL, -1, "");
   if (!pending)
     {
        ERR("Error: could not call");
        return EXIT_FAILURE;
     }
   provider = calloc(1, sizeof(Elocation_Provider));
   pending2 = edbus_proxy_call(ubuntu_geoclue, "GetProviderInfo", provider_info_cb, NULL, -1, "");
   if (!pending2)
     {
        ERR("Error: could not call");
        return EXIT_FAILURE;
     }

   /* Geoclue might automatically shutdown a provider if it is not in use. As we use signals on this
    * one we need to increase the reference count to keep it alive. */
   pending3 = edbus_proxy_call(ubuntu_geoclue, "AddReference", _reference_add_cb, NULL, -1, "");
   if (!pending3)
     {
        ERR("Error: could not call");
        return EXIT_FAILURE;
     }

   cb_address_changed = edbus_proxy_signal_handler_add(ubuntu_address, "AddressChanged", address_signal_cb, NULL);
   cb_position_changed = edbus_proxy_signal_handler_add(ubuntu_position, "PositionChanged", position_signal_cb, NULL);

   edbus_name_owner_changed_callback_add(conn, GEOCLUE_DBUS_NAME, _name_owner_changed,
                                         NULL, EINA_TRUE);

   ecore_event_handler_add(ELOCATION_EVENT_IN, geoclue_start, NULL);
   ecore_event_handler_add(ELOCATION_EVENT_OUT, geoclue_stop, NULL);

   cb_status_changed = edbus_proxy_signal_handler_add(ubuntu_geoclue, "StatusChanged", status_signal_cb, NULL);
}

EAPI void
elocation_shutdown()
{
   EDBus_Pending *pending, *pending2;

   /* To allow geoclue freeing unused providers we free our reference on it here */
   pending = edbus_proxy_call(ubuntu_geoclue, "RemoveReference", _reference_del_cb, NULL, -1, "");
   if (!pending)
     {
        ERR("Error: could not call");
     }

   pending2 = edbus_proxy_call(meta_geoclue, "RemoveReference", _reference_del_cb, NULL, -1, "");
   if (!pending2)
     {
        ERR("Error: could not call");
     }

   free(provider);
   edbus_name_owner_changed_callback_del(conn, GEOCLUE_DBUS_NAME, _name_owner_changed, NULL);
   edbus_signal_handler_unref(cb_address_changed);
   edbus_signal_handler_unref(cb_position_changed);
   edbus_signal_handler_unref(cb_status_changed);
   edbus_connection_unref(conn);
   edbus_shutdown();
   ecore_shutdown();
   eina_log_domain_unregister(_elocation_log_dom);
   eina_shutdown();
}
