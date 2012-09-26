#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>

#include <Ecore.h>
#include <E_DBus.h>

#define GEOCLUE_OBJECT_PATH "/org/freedesktop/Geoclue/Master"

#define GEOCLUE_DBUS_NAME "org.freedesktop.Geoclue.Master"
#define GEOCLE_MASTER_IFACE DEOCLUE_DBUS_NAME
#define GEOCLUE_MASTERCLIENT_IFACE "org.freedesktop.Geoclue.MasterClient"
#define GEOCLUE_IFACE "org.freedesktop.Geoclue"
#define GEOCLUE_POSITION_IFACE "org.freedesktop.Geoclue.Position"
#define GEOCLUE_ADDRESS_IFACE "org.freedesktop.Geoclue.Address"
#define GEOCLUE_VELOCITY_IFACE "org.freedesktop.Geoclue.Velocity"
#define GEOCLUE_GEOCODE_IFACE "org.freedesktop.Geoclue.Geocode"
#define GEOCLUE_REVERSEGEOCODE_IFACE "org.freedesktop.Geoclue.ReverseGeocode"

#define GSMLOC_DBUS_NAME "org.freedesktop.Geoclue.Providers.Gsmloc"
#define GSMLOC_OBJECT_PATH "/org/freedesktop/Geoclue/Providers/Gsmloc"
#define HOSTIP_DBUS_NAME "org.freedesktop.Geoclue.Providers.Hostip"
#define HOSTIP_OBJECT_PATH "/org/freedesktop/Geoclue/Providers/Hostip"
#define SKYHOOK_DBUS_NAME "org.freedesktop.Geoclue.Providers.Skyhook"
#define SKYHOOK_OBJECT_PATH "/org/freedesktop/Geoclue/Providers/Skyhook"
#define UBUNTU_DBUS_NAME "org.freedesktop.Geoclue.Providers.UbuntuGeoIP"
#define UBUNTU_OBJECT_PATH "/org/freedesktop/Geoclue/Providers/UbuntuGeoIP"

#define GEOCLUE_ADDRESS_KEY_AREA "area"
#define GEOCLUE_ADDRESS_KEY_COUNTRY "country"
#define GEOCLUE_ADDRESS_KEY_COUNTRYCODE "countrycode"
#define GEOCLUE_ADDRESS_KEY_LOCALITY "locality"
#define GEOCLUE_ADDRESS_KEY_POSTALCODE "postalcode"
#define GEOCLUE_ADDRESS_KEY_REGION "region"
#define GEOCLUE_ADDRESS_KEY_STREET "street"

typedef enum {
   GEOCLUE_POSITION_FIELDS_NONE = 0,
   GEOCLUE_POSITION_FIELDS_LATITUDE = 1 << 0,
   GEOCLUE_POSITION_FIELDS_LONGITUDE = 1 << 1,
   GEOCLUE_POSITION_FIELDS_ALTITUDE = 1 << 2
} GeocluePositionFields;

typedef enum {
   GEOCLUE_CONNECTIVITY_UNKNOWN,
   GEOCLUE_CONNECTIVITY_OFFLINE,
   GEOCLUE_CONNECTIVITY_ACQUIRING,
   GEOCLUE_CONNECTIVITY_ONLINE,
} GeoclueNetworkStatus;

typedef enum {
   GEOCLUE_ACCURACY_LEVEL_NONE = 0,
   GEOCLUE_ACCURACY_LEVEL_COUNTRY,
   GEOCLUE_ACCURACY_LEVEL_REGION,
   GEOCLUE_ACCURACY_LEVEL_LOCALITY,
   GEOCLUE_ACCURACY_LEVEL_POSTALCODE,
   GEOCLUE_ACCURACY_LEVEL_STREET,
   GEOCLUE_ACCURACY_LEVEL_DETAILED,
} GeoclueAccuracyLevel;

typedef enum {
   GEOCLUE_RESOURCE_NONE = 0,
   GEOCLUE_RESOURCE_NETWORK = 1 << 0,
   GEOCLUE_RESOURCE_CELL = 1 << 1,
   GEOCLUE_RESOURCE_GPS = 1 << 2,

   GEOCLUE_RESOURCE_ALL = (1 << 10) - 1
} GeoclueResourceFlags;

typedef enum {
   GEOCLUE_STATUS_ERROR,
   GEOCLUE_STATUS_UNAVAILABLE,
   GEOCLUE_STATUS_ACQUIRING,
   GEOCLUE_STATUS_AVAILABLE
} GeoclueStatus;

typedef enum {
   GEOCLUE_VELOCITY_FIELDS_NONE = 0,
   GEOCLUE_VELOCITY_FIELDS_SPEED = 1 << 0,
   GEOCLUE_VELOCITY_FIELDS_DIRECTION = 1 << 1,
   GEOCLUE_VELOCITY_FIELDS_CLIMB = 1 << 2
} GeoclueVelocityFields;

