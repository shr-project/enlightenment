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
#include <elocation_private.h>

/**
 * @defgroup Location_Events Available location events
 * @brief Location events that are emitted from the library
 * @since 1.8
 * @{
 */
EAPI int ELOCATION_EVENT_IN;
EAPI int ELOCATION_EVENT_OUT;
EAPI int ELOCATION_EVENT_STATUS;
EAPI int ELOCATION_EVENT_POSITION;
EAPI int ELOCATION_EVENT_ADDRESS;
/**@}*/

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
   unsigned int timestamp;
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
   GeocluePositionFields fields;
   unsigned int timestamp;
   double latitude;
   double longitude;
   double altitude;
   Elocation_Accuracy *accur;
} Elocation_Position;

/**
 * @typedef Elocation_Requirements
 * @since 1.8
 *
 * Requirement settings for the location provider. Requirements can be an level
 * of accurancy or allowed resources like network access or GPS.
 */
typedef struct _Elocation_Requirements
{
   GeoclueAccuracyLevel accurancy_level;
   int time;
   Eina_Bool require_update;
   GeoclueResourceFlags allowed_resources;
} Elocation_Requirements;

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

Eina_Bool elocation_init();
void elocation_shutdown();
#endif
