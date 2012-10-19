#ifndef _ELOCATION_H
#define _ELOCATION_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef EFL_ECORE_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif /* ! DLL_EXPORT */
# else
#  define EAPI __declspec(dllimport)
# endif /* ! EFL_ECORE_BUILD */
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EAPI __attribute__ ((visibility("default")))
#  else
#   define EAPI
#  endif
# else
#  define EAPI
# endif
#endif /* ! _WIN32 */

#include <stdio.h>

#include <Ecore.h>
#include <EDBus.h>

/**
 * @defgroup Location_Events Available location events
 * @brief Location events that are emitted from the library
 * @since 1.8
 *
 * Ecore events emitted by the library. Applications can register ecore event
 * handlers to react on such events.
 * @{
 */
EAPI extern int ELOCATION_EVENT_STATUS; /**< Status changed */
EAPI extern int ELOCATION_EVENT_POSITION; /**< Position changed */
EAPI extern int ELOCATION_EVENT_ADDRESS; /**< Address changed */
EAPI extern int ELOCATION_EVENT_VELOCITY; /**< Velocity changed */
/**@}*/

/**
 * @typedef Elocation_Accuracy_Level
 * @since 1.8
 *
 * Different location accuraccy levels from country level up to detailed,
 * e.g. GPS, information.
 */
typedef enum {
   ELOCATION_ACCURACY_LEVEL_NONE = 0,
   ELOCATION_ACCURACY_LEVEL_COUNTRY,
   ELOCATION_ACCURACY_LEVEL_REGION,
   ELOCATION_ACCURACY_LEVEL_LOCALITY,
   ELOCATION_ACCURACY_LEVEL_POSTALCODE,
   ELOCATION_ACCURACY_LEVEL_STREET,
   ELOCATION_ACCURACY_LEVEL_DETAILED,
} Elocation_Accuracy_Level;

/**
 * @typedef Elocation_Resource_Flags
 * @since 1.8
 *
 * Flags for available system resources to be used for locating.
 */
typedef enum {
   ELOCATION_RESOURCE_NONE = 0,
   ELOCATION_RESOURCE_NETWORK = 1 << 0, /**< Internet connection is avaibale */
   ELOCATION_RESOURCE_CELL = 1 << 1, /**< Cell network information, e.g. GSM, is available */
   ELOCATION_RESOURCE_GPS = 1 << 2, /**< GPS information is available */

   ELOCATION_RESOURCE_ALL = (1 << 10) - 1 /**< All resources are available */
} Elocation_Resource_Flags;

/**
 * @typedef Elocation_Accuracy
 * @since 1.8
 *
 * Information about the accurancy of the reported location.
 */
typedef struct _Elocation_Accuracy
{
   int level;
   double horizontal;
   double vertical;
} Elocation_Accuracy;

/**
 * @typedef Elocation_Address
 * @since 1.8
 *
 * Location information in textual form. Depending on the used provider this
 * can cover only the country or a detailed address based on GPS information.
 */
typedef struct _Elocation_Address
{
   unsigned int timestamp; /**< Timestamp of data read out in seconds since epoch */
   char *country;
   char *countrycode;
   char *locality;
   char *postalcode;
   char *region;
   char *timezone;
   Elocation_Accuracy *accur;
} Elocation_Address;

/**
 * @typedef Elocation_Position
 * @since 1.8
 *
 * Location information based on the GPS grid. Latitude, longitude and altitude.
 */
typedef struct _Elocation_Postion
{
   unsigned int timestamp; /**< Timestamp of data read out in seconds since epoch */
   double latitude;
   double longitude;
   double altitude;
   Elocation_Accuracy *accur;
} Elocation_Position;

/**
 * @typedef Elocation_Velocity
 * @since 1.8
 *
 */
typedef struct _Elocation_Velocity
{
   unsigned int timestamp; /**< Timestamp of data read out in seconds since epoch */
   double speed;
   double direction;
   double climb;
} Elocation_Velocity;

/**
 * @typedef Elocation_Requirements
 * @since 1.8
 *
 * Requirement settings for the location provider. Requirements can be an level
 * of accurancy or allowed resources like network access or GPS. See
 * #Elocation_Resource_Flags for all available resource flags.
 */
typedef struct _Elocation_Requirements
{
   Elocation_Accuracy_Level accurancy_level;
   int time;
   Eina_Bool require_update;
   Elocation_Resource_Flags allowed_resources;
} Elocation_Requirements;

/**
 * @brief Create a new address object to operate on.
 * @return Address object.
 *
 * @since 1.8
 */
EAPI Elocation_Address *elocation_address_new();

/**
 * @brief Free an address object
 * @param address Address object to be freed.
 *
 * @since 1.8
 */
EAPI void elocation_address_free(Elocation_Address *address);

/**
 * @brief Create a new position object to operate on.
 * @return Position object.
 *
 * @since 1.8
 */
EAPI Elocation_Position *elocation_position_new();

/**
 * @brief Free an position object
 * @param position Position object to be freed.
 *
 * @since 1.8
 */
EAPI void elocation_position_free(Elocation_Position *position);

/**
 * @brief Get the current address information.
 * @param address Address struct to be filled with information.
 * @return EINA_TRUE for success and EINA_FALSE for failure.
 *
 * @since 1.8
 */
EAPI Eina_Bool elocation_address_get(Elocation_Address *address);

/**
 * @brief Get the current position information.
 * @param position Position struct to be filled with information.
 * @return EINA_TRUE for success and EINA_FALSE for failure.
 *
 * @since 1.8
 */
EAPI Eina_Bool elocation_position_get(Elocation_Position *position);

/**
 * @brief Get the current status.
 * @param status Status
 * @return EINA_TRUE for success and EINA_FALSE for failure.
 *
 * @since 1.8
 */
EAPI Eina_Bool elocation_status_get(int *status);

/**
 * @brief Set the requirements.
 * @param requirements Requirements
 * @return EINA_TRUE for success and EINA_FALSE for failure.
 *
 * @since 1.8
 */
EAPI Eina_Bool elocation_requirements_set(Elocation_Requirements *requirements);

/**
 * @brief Convert position to address
 * @param position_shadow Position input
 * @param address_shadow Address output
 * @return EINA_TRUE for success and EINA_FALSE for failure.
 *
 * @since 1.8
 */
EAPI Eina_Bool elocation_position_to_address(Elocation_Position *position_shadow, Elocation_Address *address_shadow);

/**
 * @brief Convert address to position
 * @param address_shadow Address input
 * @param position_shadow Position output
 * @return EINA_TRUE for success and EINA_FALSE for failure.
 *
 * @since 1.8
 */
EAPI Eina_Bool elocation_address_to_position(Elocation_Address *address_shadow, Elocation_Position *position_shadow);

/**
 * @brief Convert free form address tring to position
 * @param freeform_address Address string in free form
 * @param position_shadow Position output
 * @return EINA_TRUE for success and EINA_FALSE for failure.
 *
 * @since 1.8
 */
EAPI Eina_Bool elocation_freeform_address_to_position(const char *freeform_address, Elocation_Position *position_shadow);


EAPI Eina_Bool elocation_init();
EAPI void elocation_shutdown();
#endif
