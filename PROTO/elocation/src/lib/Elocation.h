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
#include <E_DBus.h>
#include <elocation_private.h>

int E_LOCATION_EVENT_IN;
int E_LOCATION_EVENT_OUT;

typedef struct _gc_accuracy
{
   int level;
   double horizontal;
   double vertical;
} gc_accuracy;

typedef struct _gc_address
{
   unsigned int timestamp;
   char *country;
   char *countrycode;
   char *locality;
   char *postalcode;
   char *region;
   char *timezone;
   gc_accuracy accur;
} gc_address;

typedef struct _gc_postion
{
   GeocluePositionFields fields;
   unsigned int timestamp;
   double latitude;
   double longitude;
   double altitude;
   gc_accuracy accur;
} gc_postion;

typedef struct _gc_provider
{
   char *name;
   char *description;
   char *service;
   char *path;
   GeoclueStatus status;
} gc_provider;

typedef struct _gc_requirements
{
   GeoclueAccuracyLevel accurancy_level;
   int time;
   Eina_Bool require_update;
   GeoclueResourceFlags allowed_resources;
} gc_requirements;

EAPI int elocation_init(E_DBus_Connection *conn);
EAPI int elocation_shutdown(E_DBus_Connection *conn);
#endif
