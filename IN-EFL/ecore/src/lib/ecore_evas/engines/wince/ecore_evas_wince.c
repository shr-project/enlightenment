#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h> /* for NULL */

#include <Ecore.h>
#include "ecore_private.h"

#include "ecore_evas_private.h"
#include "Ecore_Evas.h"

static const char *interface_wince_name = "wince";
static const int   interface_wince_version = 1;

static Ecore_Evas_Interface_WinCE *_ecore_evas_wince_interface_new(void);

EAPI Ecore_Evas *
_ecore_evas_software_wince_new(Ecore_WinCE_Window *parent EINA_UNUSED,
                              int                 x EINA_UNUSED,
                              int                 y EINA_UNUSED,
                              int                 width EINA_UNUSED,
                              int                 height EINA_UNUSED)
{
   return NULL;
}

EAPI Ecore_Evas *
_ecore_evas_software_wince_fb_new(Ecore_WinCE_Window *parent EINA_UNUSED,
                                 int                 x EINA_UNUSED,
                                 int                 y EINA_UNUSED,
                                 int                 width EINA_UNUSED,
                                 int                 height EINA_UNUSED)
{
   return NULL;
}

EAPI Ecore_Evas *
_ecore_evas_software_wince_gapi_new(Ecore_WinCE_Window *parent EINA_UNUSED,
                                   int                 x EINA_UNUSED,
                                   int                 y EINA_UNUSED,
                                   int                 width EINA_UNUSED,
                                   int                 height EINA_UNUSED)
{
   return NULL;
}

EAPI Ecore_Evas *
_ecore_evas_software_wince_ddraw_new(Ecore_WinCE_Window *parent EINA_UNUSED,
                                    int                 x EINA_UNUSED,
                                    int                 y EINA_UNUSED,
                                    int                 width EINA_UNUSED,
                                    int                 height EINA_UNUSED)
{
   return NULL;
}

EAPI Ecore_Evas *
_ecore_evas_software_wince_gdi_new(Ecore_WinCE_Window *parent EINA_UNUSED,
                                  int                 x EINA_UNUSED,
                                  int                 y EINA_UNUSED,
                                  int                 width EINA_UNUSED,
                                  int                 height EINA_UNUSED)
{
   return NULL;
}

static Ecore_WinCE_Window *
_ecore_evas_software_wince_window_get(const Ecore_Evas *ee EINA_UNUSED)
{
   return NULL;
}

static Ecore_Evas_Interface_WinCE *
_ecore_evas_wince_interface_new(void)
{
   Ecore_Evas_Interface_WinCE *iface;

   iface = calloc(1, sizeof(Ecore_Evas_Interface_WinCE));
   if (!iface) return NULL;

   iface->base.name = interface_wince_name;
   iface->base.version = interface_wince_version;

   iface->window_get = _ecore_evas_software_wince_window_get;

   return iface;
}
