#include "e_mod_main.h"

EAPI E_Module_Api e_modapi = {E_MODULE_API_VERSION, "Sawed-Off Shotgun"};
static E_Config_DD *conf_edd = NULL;
static Eio_File *eio_file = NULL;

Mod *mod = NULL;
Config *sos_config = NULL;

const char _action_entry_toggle[] = "shotgun_entry_toggle";
const char _action_entry_next[] = "shotgun_entry_next";
const char _action_entry_prev[] = "shotgun_entry_prev";
const char _action_link_show_1[] = "shotgun_link_toggle_1";
const char _action_link_show_next[] = "shotgun_link_toggle_next";
const char _action_link_show_prev[] = "shotgun_link_toggle_prev";

static void
mod_contact_update(Mod_Contact *mc, const char *res, const char *desc, Shotgun_User_Status st, Shotgun_Presence_Type type, int priority)
{
   if (res) eina_stringshare_replace(&mc->pres.jid, res);
   if (desc) eina_stringshare_replace(&mc->pres.description, desc);
   mc->pres.status = st;
   mc->pres.priority = priority;
   mc->pres.type = type;
}

static Eina_Bool
mod_contact_update_check(Mod_Contact *mc, Shotgun_User_Status st, int priority)
{
   if (mc->pres.type == SHOTGUN_PRESENCE_TYPE_UNAVAILABLE) return EINA_TRUE;
   if (!st) return EINA_TRUE; // going to get another update immediately
   return mc->pres.priority <= priority;
}

static void
mod_contact_free(Mod_Contact *mc)
{
   if (!mc) return;
   eina_stringshare_del(mc->jid);
   eina_stringshare_del(mc->pres.jid);
   eina_stringshare_del(mc->pres.description);
   mod->contacts_list = eina_inlist_remove(mod->contacts_list, EINA_INLIST_GET(mc));
   if (mod->contact_active == mc)
     {
        E_FN_DEL(e_object_del, mod->popup);
        mod->contact_active = NULL;
     }
   free(mc);
}

// "sssui" org.shotgun.core.status(): String JID, String resource, String desc, Shotgun_User_Status st, Shotgun_Presence_Type type, int priority
static Mod_Contact *
mod_contact_new(const char *jid, const char *res, const char *desc, Shotgun_User_Status st, Shotgun_Presence_Type type, int priority)
{
   Mod_Contact *mc;

   mc = E_NEW(Mod_Contact, 1);
   mc->jid = eina_stringshare_add(jid);
   mc->pres.jid = eina_stringshare_add(res);
   mc->pres.description = eina_stringshare_add(desc);
   mc->pres.status = st;
   mc->pres.type = type;
   mc->pres.priority = priority;

   eina_hash_add(mod->contacts, jid, mc);
   mod->contacts_list = eina_inlist_prepend(mod->contacts_list, EINA_INLIST_GET(mc));
   
   return mc;
}

static void
_set_active(Eina_Bool active)
{
   mod->active = !!active;
   if (!active)
     {
        E_FN_DEL(e_object_del, mod->popup);
        mod->contact_active = NULL;
        return;
     }
}

static void
_e_mod_sos_config_free(void)
{
   E_FREE(sos_config);
}

static void
_e_mod_sos_config_load(void)
{
#undef T
#undef D
   conf_edd = E_CONFIG_DD_NEW("Config", Config);
   #define T Config
   #define D conf_edd
   E_CONFIG_VAL(D, T, config_version, UINT);

   sos_config = e_config_domain_load("module.sawed-off_shotgun", conf_edd);
   if (sos_config)
     {
        if (!e_util_module_config_check("Sawed-Off Shotgun", sos_config->config_version, MOD_CONFIG_FILE_VERSION))
          _e_mod_sos_config_free();
     }

   if (sos_config) return;
   sos_config = E_NEW(Config, 1);
   sos_config->config_version = (MOD_CONFIG_FILE_EPOCH << 16);
   sos_config->position = E_GADCON_ORIENT_CORNER_RB;
}

static void
_link_cb(void *d EINA_UNUSED, const EDBus_Message *msg)
{
   char *url;

   if (edbus_message_arguments_get(msg, "s", &url))
     mod->images = eina_list_prepend(mod->images, eina_stringshare_add(url));
}

static void
_link_self_cb(void *d EINA_UNUSED, const EDBus_Message *msg)
{
   char *url;

   if (sos_config->ignore_self_links) return;
   if (edbus_message_arguments_get(msg, "s", &url))
     mod->images = eina_list_prepend(mod->images, eina_stringshare_add(url));
}

static void
_contact_info_cb(void *d, const EDBus_Message *msg, EDBus_Pending *pending EINA_UNUSED)
{
   const char *jid = d;
   char *name, *icon;
   Shotgun_User_Status st;
   int priority;
   Mod_Contact *mc;

   if (!edbus_message_arguments_get(msg, "ssui", &name, &icon, &st, &priority))
     {
        eina_stringshare_del(jid);
        return;
     }
   mc = eina_hash_find(mod->contacts, jid);
   if (mc)
     {
        if (mod_contact_update_check(mc, st, priority))
          mod_contact_update(mc, NULL, NULL, st, 0, priority);
     }
   else
     mc = mod_contact_new(jid, NULL, NULL, st, 0, priority);
   eina_stringshare_replace(&mc->name, name);
   eina_stringshare_replace(&mc->icon, icon);
   eina_stringshare_del(jid);
}

static void
_status_cb(void *d EINA_UNUSED, const EDBus_Message *msg)
{
   char *jid, *res, *desc;
   unsigned int st, type;
   int priority;
   Mod_Contact *mc;

   if (!edbus_message_arguments_get(msg, "sssuui",
     &jid, &res, &desc, &st, &type, &priority))
     return;

   if (type == SHOTGUN_PRESENCE_TYPE_SUBSCRIBE) return;
   if (type == SHOTGUN_PRESENCE_TYPE_UNSUBSCRIBE)
     {
        eina_hash_del_by_key(mod->contacts, jid);
        return;
     }

   mc = eina_hash_find(mod->contacts, jid);
   if (!mc)
     {
        mc = mod_contact_new(jid, res, desc, st, type, priority);
        edbus_proxy_call(mod->proxy_contact, "info", _contact_info_cb, eina_stringshare_ref(mc->jid), -1, "s", mc->jid);
        return;
     }
   
   if (mod_contact_update_check(mc, st, priority))
     mod_contact_update(mc, res, desc, st, type, priority);
}

static void
_contacts_list_cb(void *d EINA_UNUSED, const EDBus_Message *msg, EDBus_Pending *pending EINA_UNUSED)
{
   EDBus_Message_Iter *array = NULL;
   char *txt = NULL;

   if (edbus_message_error_get(msg, NULL, NULL)) return;
   if (!edbus_message_arguments_get(msg, "as", &array)) return;

   while (edbus_message_iter_get_and_next(array, 's', &txt))
     edbus_proxy_call(mod->proxy_contact, "info", _contact_info_cb, eina_stringshare_add(txt), -1, "s", txt);
}

static void
_connected_cb(void *d EINA_UNUSED, const EDBus_Message *msg)
{
   Eina_Bool state;

   if (!edbus_message_arguments_get(msg, "b", &state)) return;
   mod->connected = !!state;
   _set_active((mod->nameowned && mod->connected));
   if (!mod->connected)
     {
        eina_hash_free_buckets(mod->contacts);
        return;
     }
   if (mod->contacts_list) return;
   edbus_proxy_call(mod->proxy_link, "get", _contacts_list_cb, NULL, -1, "");
}

static void
_new_msg_cb(void *d EINA_UNUSED, const EDBus_Message *msg)
{
   char *jid, *txt;
   Mod_Contact *mc;

   if (!edbus_message_arguments_get(msg, "ss", &jid, &txt)) return;
   mc = eina_hash_find(mod->contacts, jid);
   if (!mc) return; // should be impossible
   mod->contacts_list = eina_inlist_promote(mod->contacts_list, EINA_INLIST_GET(mc));
}

static void
_name_owner_change(void *d EINA_UNUSED, const char *bus EINA_UNUSED, const char *old EINA_UNUSED, const char *new)
{
   mod->nameowned = (new && new[0]);
   if (mod->nameowned) mod->connected = 1;
   _set_active((mod->nameowned && mod->connected));
   if (!mod->nameowned) E_FREE_LIST(mod->images, eina_stringshare_del);
}

static void
_link_activate(unsigned int num)
{
   const char *url;

   if (mod->tooltip_active)
     {
        edbus_proxy_call(mod->proxy_link, "show", NULL, NULL, -1, "s", "");
        mod->tooltip_number = mod->tooltip_active = 0;
        return;
     }
   if (!mod->images) return;
   url = eina_list_nth(mod->images, num);
   if (!url) url = eina_list_data_get(mod->images);
   mod->tooltip_number = num;
   mod->tooltip_active = 1;
   edbus_proxy_call(mod->proxy_link, "show", NULL, NULL, -1, "s", url);
}

static void
_sawedoff_activate_send(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *obj, Evas_Event_Key_Down *ev)
{
   if (strcmp(ev->keyname, "Return") && strcmp(ev->keyname, "KP_Enter")) return;
   edbus_proxy_call(mod->proxy_contact, "send_echo", NULL, NULL, -1, "ssu",
     mod->contact_active->jid, e_entry_text_get(obj), 0);
   e_entry_clear(mod->popup_entry);
}

static void
_sawedoff_activate(Mod_Contact *mc)
{
   Evas_Object *o, *entry, *img;
   int x, y, mw, mh;
   E_Zone *zone;

   mod->contact_active = mc;
   zone = e_util_zone_current_get(e_manager_current_get());
   if (mod->popup)
     {
        o = mod->popup_bg, entry = mod->popup_entry, img = mod->popup_img;
        e_entry_clear(entry);
     }
   else
     {
        mod->popup = e_popup_new(zone, 0, 0, 1, 1);

        mod->popup_bg = o = edje_object_add(mod->popup->evas);
        if (!e_theme_edje_object_set(o, "base/theme/shotgun", "e/shotgun/module/sawedoff"))
          edje_object_file_set(o, mod->edj, "e/shotgun/module/sawedoff");
        e_popup_edje_bg_object_set(mod->popup, o);

        mod->popup_entry = entry = e_entry_add(mod->popup->evas);
        edje_object_part_swallow(o, "shotgun.swallow.entry", entry);
        1 | evas_object_key_grab(entry, "Return", 0, 0, EINA_TRUE);
        1 | evas_object_key_grab(entry, "KP_Enter", 0, 0, EINA_TRUE);
        evas_object_event_callback_add(entry, EVAS_CALLBACK_KEY_DOWN, (Evas_Object_Event_Cb)_sawedoff_activate_send, NULL);

        mod->popup_img = img = e_icon_add(mod->popup->evas);
     }
   e_entry_focus(entry);
   edje_object_part_text_set(o, "shotgun.text", mc->name);
   if (mc->icon)
     e_icon_file_key_set(img, mod->edj, mc->icon);
   else
     e_icon_fdo_icon_set(img, "shotgun");
   edje_object_part_swallow(o, "shotgun.swallow.icon", img);

   edje_object_size_min_calc(o, &mw, &mh);
   x = y = 0;
   switch (sos_config->position)
     {
      case E_GADCON_ORIENT_FLOAT: // center
        x = (zone->w - mw) / 2;
        y = (zone->h - mh) / 2;
        break;

      case E_GADCON_ORIENT_HORIZ:
        if (sos_config->fill_side) mw = zone->w;
        else x = (zone->w - mw) / 2;
        break;

      case E_GADCON_ORIENT_VERT:
        if (sos_config->fill_side) mh = zone->h;
        else y = (zone->h - mh) / 2;
        break;

      case E_GADCON_ORIENT_LEFT:
        if (sos_config->fill_side) mh = zone->h;
        else y = (zone->h - mh) / 2;
        break;

      case E_GADCON_ORIENT_RIGHT:
        if (sos_config->fill_side) mh = zone->h;
        else y = (zone->h - mh) / 2;
        x = zone->w - mw;
        break;

      case E_GADCON_ORIENT_TOP:
        if (sos_config->fill_side) mw = zone->w;
        else x = (zone->w - mw) / 2;
        break;

      case E_GADCON_ORIENT_BOTTOM:
        if (sos_config->fill_side) mw = zone->w;
        else x = (zone->w - mw) / 2;
        y = zone->h - mh;
        break;

      case E_GADCON_ORIENT_CORNER_TL:
        if (sos_config->fill_side) mw = zone->w;
        break;

      case E_GADCON_ORIENT_CORNER_TR:
        if (sos_config->fill_side) mw = zone->w;
        else x = zone->w - mw;
        break;

      case E_GADCON_ORIENT_CORNER_BL:
        if (sos_config->fill_side) mw = zone->w;
        y = zone->h - mh;
        break;

      case E_GADCON_ORIENT_CORNER_BR:
        if (sos_config->fill_side) mw = zone->w;
        else x = zone->w - mw;
        y = zone->h - mh;
        break;

      case E_GADCON_ORIENT_CORNER_LT:
        if (sos_config->fill_side) mh = zone->h;
        break;

      case E_GADCON_ORIENT_CORNER_RT:
        if (sos_config->fill_side) mh = zone->h;
        x = zone->w - mw;
        break;

      case E_GADCON_ORIENT_CORNER_LB:
        if (sos_config->fill_side) mh = zone->h;
        else y = zone->h - mh;
        break;

      case E_GADCON_ORIENT_CORNER_RB:
        if (sos_config->fill_side) mh = zone->h;
        else y = zone->h - mh;
        x = zone->w - mw;
        break;

      default:
        break;
     }
   evas_object_show(o);
   e_popup_move_resize(mod->popup, x, y, mw, mh);
   evas_object_resize(o, mw, mh);
   e_popup_show(mod->popup);
}

static void
_action_entry_next_cb(E_Object *obj EINA_UNUSED, const char *params EINA_UNUSED)
{
   Mod_Contact *mc;

   if (!mod->active) return;
   if (!mod->contacts_list) return;
   if (mod->contact_active)
     {
        if (EINA_INLIST_GET(mod->contact_active)->next)
          mc = EINA_INLIST_CONTAINER_GET(EINA_INLIST_GET(mod->contact_active)->next, Mod_Contact);
        else
          mc = EINA_INLIST_CONTAINER_GET(EINA_INLIST_GET(mod->contact_active), Mod_Contact);
     }
   else
     mc = EINA_INLIST_CONTAINER_GET(mod->contacts_list, Mod_Contact);
   _sawedoff_activate(mc);
}

static void
_action_entry_prev_cb(E_Object *obj EINA_UNUSED, const char *params EINA_UNUSED)
{
   Mod_Contact *mc;

   if (!mod->active) return;
   if (!mod->contacts_list) return;
   if (mod->contact_active)
     {
        if (EINA_INLIST_GET(mod->contact_active)->prev)
          mc = EINA_INLIST_CONTAINER_GET(EINA_INLIST_GET(mod->contact_active)->prev, Mod_Contact);
        else
          mc = EINA_INLIST_CONTAINER_GET(EINA_INLIST_GET(mod->contact_active)->last, Mod_Contact);
     }
   else
     mc = EINA_INLIST_CONTAINER_GET(mod->contacts_list->last, Mod_Contact);
   _sawedoff_activate(mc);
}

static void
_action_entry_toggle_cb(E_Object *obj EINA_UNUSED, const char *params EINA_UNUSED)
{
   if (mod->popup)
     {
        e_popup_hide(mod->popup);
        return;
     }
   if (!mod->active) return;
   _sawedoff_activate(EINA_INLIST_CONTAINER_GET(mod->contacts_list, Mod_Contact));
}

static void
_action_link_show_1_cb(E_Object *obj EINA_UNUSED, const char *params EINA_UNUSED)
{
   _link_activate(0);
}

static void
_action_link_show_prev_cb(E_Object *obj EINA_UNUSED, const char *params EINA_UNUSED)
{
   if (mod->tooltip_active)
     {
        mod->tooltip_active = 0;
        _link_activate(mod->tooltip_number - 1);
     }
   else
     _link_activate(eina_list_count(mod->images) - 1);
}

static void
_action_link_show_next_cb(E_Object *obj EINA_UNUSED, const char *params EINA_UNUSED)
{
   if (mod->tooltip_active)
     {
        mod->tooltip_active = 0;
        _link_activate(mod->tooltip_number + 1);
     }
   else
     _link_activate(0);
}

static void
_eio_error_cb(void *d EINA_UNUSED, Eio_File *f EINA_UNUSED, int x EINA_UNUSED)
{
   eio_file = NULL;
}

static void
_eio_open_cb(void *d EINA_UNUSED, Eio_File *f EINA_UNUSED, Eet_File *ef)
{
   mod->ef = ef;
   eio_file = NULL;
}

EAPI void *
e_modapi_init(E_Module *m)
{
   char buf[4096];
   EDBus_Connection *conn;
   EDBus_Object *obj;
   EDBus_Proxy *proxy;
   E_Action *act;

   snprintf(buf, sizeof(buf), "%s/locale", m->dir);
   bindtextdomain(PACKAGE, buf);
   bind_textdomain_codeset(PACKAGE, "UTF-8");

   snprintf(buf, sizeof(buf), "%s/.config/shotgun/shotgun.eet", e_user_homedir_get());
   eio_file = eio_eet_open(buf, EET_FILE_MODE_READ, _eio_open_cb, _eio_error_cb, NULL);
   snprintf(buf, sizeof(buf), "%s/e-module-sawed-off.edj", m->dir);
   e_configure_registry_category_add("extensions", 80, D_("Extensions"),
                                     NULL, "preferences-extensions");
   e_configure_registry_item_add("extensions/sawed-off_shotgun", 110, D_("Shotgun: Sawed-Off"),
                                 NULL, buf, e_int_config_sos);

   edbus_init();
   conn = edbus_connection_get(EDBUS_CONNECTION_TYPE_SESSION);
   edbus_name_owner_changed_callback_add(conn, SHOTGUN_DBUS_METHOD_BASE, _name_owner_change, NULL, EINA_TRUE);
   obj = edbus_object_get(conn, SHOTGUN_DBUS_INTERFACE, SHOTGUN_DBUS_PATH);
   proxy = edbus_proxy_get(obj, SHOTGUN_DBUS_METHOD_BASE ".core");

   edbus_proxy_signal_handler_add(proxy, "link", _link_cb, NULL);
   edbus_proxy_signal_handler_add(proxy, "link_self", _link_self_cb, NULL);
   edbus_proxy_signal_handler_add(proxy, "new_msg", _new_msg_cb, NULL);
   edbus_proxy_signal_handler_add(proxy, "status", _status_cb, NULL);
   edbus_proxy_signal_handler_add(proxy, "connected", _connected_cb, NULL);

   mod = E_NEW(Mod, 1);
   mod->module = m;
   mod->conn = conn;
   mod->edj = eina_stringshare_add(buf);

   mod->proxy_link = edbus_proxy_get(obj, SHOTGUN_DBUS_METHOD_BASE ".link");
   mod->proxy_contact = edbus_proxy_get(obj, SHOTGUN_DBUS_METHOD_BASE ".contact");
   mod->proxy_list = edbus_proxy_get(obj, SHOTGUN_DBUS_METHOD_BASE ".list");

   _e_mod_sos_config_load();

   act = e_action_add(_action_entry_toggle);
   act->func.go = _action_entry_toggle_cb;
   e_action_predef_name_set("Shotgun : Sawed-Off", "Toggle Reply Entry", _action_entry_toggle, NULL, NULL, 0);
   mod->actions = eina_list_append(mod->actions, act);
   act = e_action_add(_action_entry_next);
   act->func.go = _action_entry_next_cb;
   e_action_predef_name_set("Shotgun : Sawed-Off", "Next Reply Entry", _action_entry_next, NULL, NULL, 0);
   mod->actions = eina_list_append(mod->actions, act);
   act = e_action_add(_action_entry_prev);
   act->func.go = _action_entry_prev_cb;
   e_action_predef_name_set("Shotgun : Sawed-Off", "Toggle Reply Entry", _action_entry_prev, NULL, NULL, 0);
   mod->actions = eina_list_append(mod->actions, act);

   act = e_action_add(_action_link_show_1);
   act->func.go = _action_link_show_1_cb;
   e_action_predef_name_set("Shotgun : Sawed-Off", "Toggle First Link", _action_link_show_1, NULL, NULL, 0);
   mod->actions = eina_list_append(mod->actions, act);
   act = e_action_add(_action_link_show_next);
   act->func.go = _action_link_show_next_cb;
   e_action_predef_name_set("Shotgun : Sawed-Off", "Show Next Link", _action_link_show_next, NULL, NULL, 0);
   mod->actions = eina_list_append(mod->actions, act);
   act = e_action_add(_action_link_show_prev);
   act->func.go = _action_link_show_prev_cb;
   e_action_predef_name_set("Shotgun : Sawed-Off", "Show Previous Link", _action_link_show_prev, NULL, NULL, 0);
   mod->actions = eina_list_append(mod->actions, act);
   
   mod->contacts = eina_hash_string_superfast_new((Eina_Free_Cb)mod_contact_free);

   return m;
}

EAPI int
e_modapi_shutdown(E_Module *m EINA_UNUSED)
{
   e_configure_registry_item_del("extensions/sawed-off_shotgun");

   e_configure_registry_category_del("extensions");

   E_FN_DEL(e_object_del, mod->cfd);
   e_config_domain_save("module.sawed-off_shotgun", conf_edd, sos_config);
   _e_mod_sos_config_free();
   E_CONFIG_DD_FREE(conf_edd);
   edbus_connection_unref(mod->conn);
   E_FREE_LIST(mod->images, eina_stringshare_del);
   E_FREE_LIST(mod->actions, e_action_del);
   eina_hash_free(mod->contacts);
   if (mod->ef) eet_close(mod->ef);
   E_FREE(mod);
   E_FN_DEL(eio_file_cancel, eio_file);
   edbus_shutdown();
   return 1;
}

EAPI int
e_modapi_save(E_Module *m EINA_UNUSED)
{
   e_config_domain_save("module.sawed-off_shotgun", conf_edd, sos_config);
   return 1;
}

