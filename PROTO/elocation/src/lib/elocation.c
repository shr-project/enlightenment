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
static Elocation_Provider *address_provider = NULL;
static Elocation_Provider *position_provider = NULL;
static EDBus_Object *obj_meta = NULL;
static EDBus_Proxy *meta_geoclue = NULL;
static EDBus_Proxy *meta_address = NULL;
static EDBus_Proxy *meta_position = NULL;
static EDBus_Proxy *meta_masterclient = NULL;
static EDBus_Proxy *meta_velocity = NULL;
static EDBus_Proxy *meta_geocode = NULL;
static EDBus_Proxy *meta_rgeocode = NULL;
static Elocation_Address *address = NULL;
static Elocation_Position *position = NULL;
static Elocation_Velocity *velocity = NULL;
static int *status = 0;

int _elocation_log_dom = -1;

EAPI int ELOCATION_EVENT_IN;
EAPI int ELOCATION_EVENT_OUT;
EAPI int ELOCATION_EVENT_STATUS;
EAPI int ELOCATION_EVENT_POSITION;
EAPI int ELOCATION_EVENT_ADDRESS;
EAPI int ELOCATION_EVENT_VELOCITY;

static void
_dummy_free(void *user_data, void *func_data)
{
   /* Don't free the event data after dispatching the event. We keep track of
    * it on our own */
}

static void
unmarshall_provider(const EDBus_Message *reply, Elocation_Provider *provider)
{
   char *name = NULL, *desc = NULL, *service = NULL, *path = NULL;

   if (!edbus_message_arguments_get(reply, "ssss", &name, &desc, &service, &path)) return;
   provider->name = strdup(name);
   provider->description = strdup(desc);
   provider->service = strdup(service);
   provider->path = strdup(path);
}

static void
meta_address_provider_info_cb(void *data, const EDBus_Message *reply, EDBus_Pending *pending)
{
   const char *err, *errmsg;

   if (edbus_message_error_get(reply, &err, &errmsg))
     {
        ERR("Error: %s %s", err, errmsg);
        return;
     }

   unmarshall_provider(reply, address_provider);
   DBG("Meta address provider name: %s, %s, %s, %s", address_provider->name,
                                                     address_provider->description,
                                                     address_provider->service,
                                                     address_provider->path);
}

static void
meta_position_provider_info_cb(void *data, const EDBus_Message *reply, EDBus_Pending *pending)
{
   const char *err, *errmsg;

   if (edbus_message_error_get(reply, &err, &errmsg))
     {
        ERR("Error: %s %s", err, errmsg);
        return;
     }

   unmarshall_provider(reply, position_provider);
   DBG("Meta position provider name: %s, %s, %s, %s", position_provider->name,
                                                      position_provider->description,
                                                      position_provider->service,
                                                      position_provider->path);
}

static void
meta_address_provider_info_signal_cb(void *data, const EDBus_Message *reply)
{
   unmarshall_provider(reply, address_provider);
   DBG("Meta address provider name changed: %s, %s, %s, %s", address_provider->name,
                                                             address_provider->description,
                                                             address_provider->service,
                                                             address_provider->path);
}

static void
meta_position_provider_info_signal_cb(void *data, const EDBus_Message *reply)
{
   unmarshall_provider(reply, position_provider);
   DBG("Meta position provider name changed: %s, %s, %s, %s", position_provider->name,
                                                              position_provider->description,
                                                              position_provider->service,
                                                              position_provider->path);
}

static void
rgeocode_cb(void *data, const EDBus_Message *reply, EDBus_Pending *pending)
{
   EDBus_Message_Iter *sub, *dict, *entry;
   const char *country = NULL, *countrycode = NULL, *locality = NULL, *postalcode = NULL;
   const char  *region = NULL, *timezone = NULL;
   int32_t level;
   double horizontal;
   double vertical;
   const char *key;
   char *value;
   Elocation_Address *addr;
   const char *err, *errmsg;

   addr = alloca(sizeof(Elocation_Address));
   addr->accur = alloca(sizeof(Elocation_Accuracy));

   if (edbus_message_error_get(reply, &err, &errmsg))
     {
        ERR("Error: %s %s", err, errmsg);
        return;
     }

   if (!edbus_message_arguments_get(reply, "a{ss}(idd)", &dict, &sub))
     return;

   while (edbus_message_iter_get_and_next(dict, 'e', &entry))
    {
       edbus_message_iter_arguments_get(entry, "ss", &key, &value);

       if (!strcmp(key, "country"))
         {
            addr->country = strdup(value);
         }
       else if (!strcmp(key, "countrycode"))
         {
            addr->countrycode = strdup(value);
         }
       else if (!strcmp(key, "locality"))
         {
            addr->locality = strdup(value);
         }
       else if (!strcmp(key, "postalcode"))
         {
            addr->postalcode = strdup(value);
         }
       else if (!strcmp(key, "region"))
         {
            addr->region = strdup(value);
         }
       else if (!strcmp(key, "timezone"))
         {
            addr->timezone = strdup(value);
         }
    }

   edbus_message_iter_arguments_get(sub, "idd", &level, &horizontal, &vertical);
   addr->accur->level = level;
   addr->accur->horizontal = horizontal;
   addr->accur->vertical = vertical;
}

static void
geocode_cb(void *data, const EDBus_Message *reply, EDBus_Pending *pending)
{
   GeocluePositionFields fields;
   int32_t level;
   double horizontal = 0.0;
   double vertical = 0.0;
   double latitude = 0.0;
   double longitude = 0.0;
   double altitude = 0.0;
   EDBus_Message_Iter *sub;
   Elocation_Position *pos;
   const char *err, *errmsg;

   pos = alloca(sizeof(Elocation_Position));
   pos->accur = alloca(sizeof(Elocation_Accuracy));

   if (edbus_message_error_get(reply, &err, &errmsg))
     {
        ERR("Error: %s %s", err, errmsg);
        return;
     }

   if (!edbus_message_arguments_get(reply, "iddd(idd)", &fields,&latitude,
                                    &longitude, &altitude, &sub))
     return;

   /* GeoClue uses some flags to mark position fields as valid. We set invalid
    * fields to 0.0 */
   if (fields & GEOCLUE_POSITION_FIELDS_LATITUDE)
      pos->latitude = latitude;
   else
      pos->latitude = 0.0;

   if (fields & GEOCLUE_POSITION_FIELDS_LONGITUDE)
      pos->longitude = longitude;
   else
      pos->longitude = 0.0;

   if (fields & GEOCLUE_POSITION_FIELDS_ALTITUDE)
      pos->altitude = altitude;
   else
      pos->altitude = 0.0;

   edbus_message_iter_arguments_get(sub, "idd", &level, &horizontal, &vertical);
   pos->accur->level = level;
   pos->accur->horizontal = horizontal;
   pos->accur->vertical = vertical;
}

static void
unmarshall_address(const EDBus_Message *reply)
{
   int32_t level, timestamp;
   EDBus_Message_Iter *sub, *dict, *entry;
   double horizontal;
   double vertical;
   const char *key;
   char *value;

   if (!edbus_message_arguments_get(reply, "ia{ss}(idd)", &timestamp, &dict, &sub))
     return;

   address->timestamp = timestamp;

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

   edbus_message_iter_arguments_get(sub, "idd", &level, &horizontal, &vertical);
   address->accur->level = level;
   address->accur->horizontal = horizontal;
   address->accur->vertical = vertical;
}

static void
address_cb(void *data, const EDBus_Message *reply, EDBus_Pending *pending)
{
   const char *err, *errmsg;

   if (edbus_message_error_get(reply, &err, &errmsg))
     {
        ERR("Error: %s %s", err, errmsg);
        return;
     }

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
unmarshall_velocity(const EDBus_Message *reply)
{
   GeoclueVelocityFields fields;
   int32_t timestamp = 0;
   double speed = 0.0;
   double direction = 0.0;
   double climb = 0.0;

   if (!edbus_message_arguments_get(reply, "iiddd", &fields, &timestamp,
                                    &speed, &direction, &climb))
     return;

   velocity->timestamp = timestamp;

   /* GeoClue uses some flags to mark velocity fields as valid. We set invalid
    * fields to 0.0 */
   if (fields & GEOCLUE_VELOCITY_FIELDS_SPEED)
      velocity->speed = speed;
   else
      velocity->speed = 0.0;

   if (fields & GEOCLUE_VELOCITY_FIELDS_DIRECTION)
      velocity->direction = direction;
   else
      velocity->direction = 0.0;

   if (fields & GEOCLUE_VELOCITY_FIELDS_CLIMB)
      velocity->climb = climb;
   else
      velocity->climb = 0.0;
}

static void
velocity_cb(void *data, const EDBus_Message *reply, EDBus_Pending *pending)
{
   const char *err, *errmsg;

   if (edbus_message_error_get(reply, &err, &errmsg))
     {
        ERR("Error: %s %s", err, errmsg);
        return;
     }

   unmarshall_velocity(reply);
   ecore_event_add(ELOCATION_EVENT_VELOCITY, velocity, _dummy_free, NULL);
}

static void
velocity_signal_cb(void *data, const EDBus_Message *reply)
{
   unmarshall_velocity(reply);
   ecore_event_add(ELOCATION_EVENT_VELOCITY, velocity, _dummy_free, NULL);
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
   EDBus_Message_Iter *sub;

   if (!edbus_message_arguments_get(reply, "iiddd(idd)", &fields, &timestamp,
                                    &latitude, &longitude, &altitude, &sub))
     return;

   edbus_message_iter_arguments_get(sub, "idd", &level, &horizontal, &vertical);

   position->timestamp = timestamp;

   /* GeoClue uses some flags to mark position fields as valid. We set invalid
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
   const char *err, *errmsg;

   if (edbus_message_error_get(reply, &err, &errmsg))
     {
        ERR("Error: %s %s", err, errmsg);
        return;
     }

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
   const char *err, *errmsg;

   if (edbus_message_error_get(reply, &err, &errmsg))
     {
        ERR("Error: %s %s", err, errmsg);
        return;
     }

   DBG("Reference added");
}

static void
_reference_del_cb(void *data, const EDBus_Message *reply, EDBus_Pending *pending)
{
   const char *err, *errmsg;

   if (edbus_message_error_get(reply, &err, &errmsg))
     {
        ERR("Error: %s %s", err, errmsg);
        return;
     }

   DBG("Reference removed");
}

static void
status_cb(void *data, const EDBus_Message *reply, EDBus_Pending *pending)
{
   const char *err, *errmsg;

   if (edbus_message_error_get(reply, &err, &errmsg))
     {
        ERR("Error: %s %s", err, errmsg);
        return;
     }

   /* We need this to be malloced to be passed to ecore_event_add. Or provide a dummy free callback. */
   status = malloc(sizeof(*status));

   if (!edbus_message_arguments_get(reply,"i",  status)) return;

   address_provider->status = position_provider->status = *status;
   ecore_event_add(ELOCATION_EVENT_STATUS, status, NULL, NULL);
}

static void
status_signal_cb(void *data, const EDBus_Message *reply)
{
   /* We need this to be malloced to be passed to ecore_event_add. Or provide a dummy free callback. */
   status = malloc(sizeof(*status));

   if (!edbus_message_arguments_get(reply,"i",  status)) return;

   address_provider->status = position_provider->status = *status;
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
   Eina_Bool updates;
   int accur_level, time, resources;
   const char *err, *errmsg;

   if (edbus_message_error_get(reply, &err, &errmsg))
     {
        ERR("Error: %s %s", err, errmsg);
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

   meta_velocity = edbus_proxy_get(obj_meta, GEOCLUE_VELOCITY_IFACE);
   if (!meta_velocity)
     {
        ERR("Error: could not get proxy for velocity");
        return;
     }

   meta_geocode = edbus_proxy_get(obj_meta, GEOCLUE_GEOCODE_IFACE);
   if (!meta_geocode)
     {
        ERR("Error: could not get proxy for geocode");
        return;
     }

   meta_rgeocode = edbus_proxy_get(obj_meta, GEOCLUE_REVERSEGEOCODE_IFACE);
   if (!meta_rgeocode)
     {
        ERR("Error: could not get proxy for reverse geocode");
        return;
     }

   /* Send Geoclue a set of requirements we have for the provider and start the address and position
    * meta provider afterwards. After this we should be ready for operation. */
   updates = EINA_FALSE; /* Especially the web providers do not offer updates */
   accur_level = ELOCATION_ACCURACY_LEVEL_COUNTRY;
   time = 0; /* Still need to figure out what this is used for */
   resources = ELOCATION_RESOURCE_ALL;

   edbus_proxy_signal_handler_add(meta_masterclient, "AddressProviderChanged", meta_address_provider_info_signal_cb, NULL);
   edbus_proxy_signal_handler_add(meta_masterclient, "PositionProviderChanged", meta_position_provider_info_signal_cb, NULL);

   if (!edbus_proxy_call(meta_masterclient, "SetRequirements", _dummy_cb, NULL, -1, "iibi", accur_level, time, updates, resources))
     {
        ERR("Error: could not call");
        return;
     }

   if (!edbus_proxy_call(meta_masterclient, "AddressStart", _dummy_cb, NULL, -1, ""))
     {
        ERR("Error: could not call");
        return;
     }

   if (!edbus_proxy_call(meta_masterclient, "PositionStart", _dummy_cb, NULL, -1, ""))
     {
        ERR("Error: could not call");
        return;
     }

   if (!edbus_proxy_call(meta_geoclue, "AddReference", _reference_add_cb, NULL, -1, ""))
     {
        ERR("Error: could not call");
        return;
     }

   if (!edbus_proxy_call(meta_address, "GetAddress", address_cb, NULL, -1, ""))
     {
        ERR("Error: could not call");
        return;
     }

   if (!edbus_proxy_call(meta_position, "GetPosition", position_cb, NULL, -1, ""))
     {
        ERR("Error: could not call");
        return;
     }

   if (!edbus_proxy_call(meta_geoclue, "GetStatus", status_cb, NULL, -1, ""))
     {
        ERR("Error: could not call");
        return;
     }

   if (!edbus_proxy_call(meta_velocity, "GetVelocity", velocity_cb, NULL, -1, ""))
     {
        ERR("Error: could not call GetVelocity");
        return;
     }

   if (!edbus_proxy_call(meta_masterclient, "GetAddressProvider", meta_address_provider_info_cb, NULL, -1, ""))
     {
        ERR("Error: could not call");
        return;
     }

   if (!edbus_proxy_call(meta_masterclient, "GetPositionProvider", meta_position_provider_info_cb, NULL, -1, ""))
     {
        ERR("Error: could not call");
        return;
     }

   edbus_proxy_signal_handler_add(meta_address, "AddressChanged", address_signal_cb, NULL);
   edbus_proxy_signal_handler_add(meta_position, "PositionChanged", position_signal_cb, NULL);
   edbus_proxy_signal_handler_add(meta_geoclue, "StatusChanged", status_signal_cb, NULL);
   edbus_proxy_signal_handler_add(meta_velocity, "VelocityChanged", velocity_signal_cb, NULL);
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

Eina_Bool
elocation_position_to_address(Elocation_Position *position_shadow, Elocation_Address *address_shadow)
{
   EDBus_Pending *pending1;

   //FIXME create message with position values

   pending1 = edbus_proxy_call(meta_rgeocode, "PositionToAddress", rgeocode_cb, NULL, -1, "");
   if (!pending1)
     {
        ERR("Error: could not call");
        return EINA_FALSE;
     }
   return EINA_TRUE;
}

Eina_Bool
elocation_address_to_position(Elocation_Address *address_shadow, Elocation_Position *position_shadow)
{
   EDBus_Pending *pending1;

   //FIXME create message with address values

   pending1 = edbus_proxy_call(meta_geocode, "AddressToPosition", geocode_cb, NULL, -1, "");
   if (!pending1)
     {
        ERR("Error: could not call");
        return EINA_FALSE;
     }
   return EINA_TRUE;
}

Eina_Bool
elocation_freeform_address_to_position(const char *freeform_address, Elocation_Position *position_shadow)
{
   EDBus_Pending *pending1;

   //FIXME create message with address

   pending1 = edbus_proxy_call(meta_geocode, "FreeformAddressToPosition", geocode_cb, NULL, -1, "");
   if (!pending1)
     {
        ERR("Error: could not call");
        return EINA_FALSE;
     }
   return EINA_TRUE;
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
elocation_status_get(int *status_shadow)
{
   if (!status) return EINA_FALSE;

   status_shadow = status;
   return EINA_TRUE;
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

   address_provider = calloc(1, sizeof(Elocation_Provider));
   position_provider = calloc(1, sizeof(Elocation_Provider));

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

   if (ELOCATION_EVENT_VELOCITY == 0)
      ELOCATION_EVENT_VELOCITY = ecore_event_type_new();

   obj_master= edbus_object_get(conn, GEOCLUE_DBUS_NAME, GEOCLUE_OBJECT_PATH);
   if (!obj_master)
     {
        ERR("Error: could not get object");
        return EXIT_FAILURE;
     }

   manager_master = edbus_proxy_get(obj_master, GEOCLUE_MASTER_IFACE);
   if (!manager_master)
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

   edbus_name_owner_changed_callback_add(conn, GEOCLUE_DBUS_NAME, _name_owner_changed,
                                         NULL, EINA_TRUE);

   ecore_event_handler_add(ELOCATION_EVENT_IN, geoclue_start, NULL);
   ecore_event_handler_add(ELOCATION_EVENT_OUT, geoclue_stop, NULL);
}

EAPI void
elocation_shutdown()
{
   EDBus_Pending *pending, *pending2;

   /* To allow geoclue freeing unused providers we free our reference on it here */
   pending2 = edbus_proxy_call(meta_geoclue, "RemoveReference", _reference_del_cb, NULL, -1, "");
   if (!pending2)
     {
        ERR("Error: could not call");
     }

   free(address_provider->name);
   free(address_provider->description);
   free(address_provider->service);
   free(address_provider->path);
   free(address_provider);

   free(position_provider->name);
   free(position_provider->description);
   free(position_provider->service);
   free(position_provider->path);
   free(position_provider);

   edbus_name_owner_changed_callback_del(conn, GEOCLUE_DBUS_NAME, _name_owner_changed, NULL);
   edbus_connection_unref(conn);
   edbus_shutdown();
   ecore_shutdown();
   eina_log_domain_unregister(_elocation_log_dom);
   eina_shutdown();
}
