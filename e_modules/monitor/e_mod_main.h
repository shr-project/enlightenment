#ifndef E_MOD_MAIN_H
#define E_MOD_MAIN_H

#include <e.h>
#include <Ecore.h>
#include "config.h"

#include "flow_chart.h"
#include "linux_2.6.h"

#define MONITOR_CONFIG_VERSION 100

typedef struct _Config      Config;
typedef struct _Config_Face Config_Face;
typedef struct _Monitor       Monitor;
typedef struct _Monitor_Face  Monitor_Face;

struct _Config
{
   Evas_List *faces;
};

struct _Config_Face
{
   int config_version;
   unsigned char enabled;

   double cpu_interval;
   double mem_interval;
   double net_interval;
   double wlan_interval;
   
   char *net_interface;
   char *wlan_interface;

   int mem_real_ignore_cached;
   int mem_real_ignore_buffers;
};

struct _Monitor
{
   Evas_List   *faces;
   E_Menu      *config_menu;
   
   Config      *conf;
};

struct _Monitor_Face
{
  Monitor *mon;
  E_Container *con;
  E_Menu      *menu;
   Config_Face *conf;

  Evas_Object *cpu, *net, *mem, *wlan;
  Evas_Object *cpu_ev_obj, *net_ev_obj, *mem_ev_obj, *wlan_ev_obj;
  Evas_Object *table_object, *monitor_object;

  Chart_Container *chart_cpu, *chart_net, *chart_mem, *chart_wlan;

  E_Gadman_Client *gmc;
};

extern E_Module_Api e_modapi;

EAPI void *e_modapi_init     (E_Module *m);
EAPI int   e_modapi_shutdown (E_Module *m);
EAPI int   e_modapi_save     (E_Module *m);
EAPI int   e_modapi_info     (E_Module *m);
EAPI int   e_modapi_about    (E_Module *m);
EAPI int   e_modapi_config   (E_Module *m);

EAPI void _monitor_cb_config_updated(void *data);
#endif
