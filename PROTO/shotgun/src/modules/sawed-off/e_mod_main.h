#ifndef E_MOD_MAIN_H
#define E_MOD_MAIN_H

#include <e.h>
#include "ui.h"
#include <EDBus.h>

#ifdef ENABLE_NLS
# include <libintl.h>
# define D_(string) dgettext(PACKAGE, string)
#else
# define bindtextdomain(domain,dir)
# define bind_textdomain_codeset(domain,codeset)
# define D_(string) (string)
#endif

#define MOD_CONFIG_FILE_EPOCH 0x0001
#define MOD_CONFIG_FILE_GENERATION 0x02
#define MOD_CONFIG_FILE_VERSION \
   ((MOD_CONFIG_FILE_EPOCH << 16) | MOD_CONFIG_FILE_GENERATION)

typedef struct Mod_Contact
{
   EINA_INLIST;
   const char *jid;
   const char *name; // display name
   const char *icon; // icon key from eet file
   /* for this module, pres.jid is the resource instead of the full jid */
   Shotgun_Event_Presence pres;
} Mod_Contact;

typedef struct Mod
{
   E_Config_Dialog *cfd;
   E_Module *module;
   Eet_File *ef;
   const char *edj;
   EDBus_Connection *conn;
   EDBus_Proxy *proxy_link;
   EDBus_Proxy *proxy_contact;
   EDBus_Proxy *proxy_list;
   Eina_List *images; /* Eina_Stringshare */
   Eina_List *actions;
   Eina_Hash *contacts;
   Mod_Contact *contact_active;
   Eina_Inlist *contacts_list; /* Eina_Stringshare */
   E_Popup *popup;
   Evas_Object *popup_bg, *popup_entry, *popup_img;
   unsigned int tooltip_number;
   Eina_Bool tooltip_active : 1;
   Eina_Bool active : 1;
   Eina_Bool nameowned : 1;
   Eina_Bool connected : 1;
} Mod;

typedef struct Config
{
   unsigned int config_version;
   E_Gadcon_Orient position;
   Eina_Bool ignore_self_links;
   Eina_Bool set_last_active;
   Eina_Bool fill_side;
} Config;

extern Mod *mod;
extern Config *sos_config;

E_Config_Dialog *e_int_config_sos(E_Container *con, const char *params);

#endif
