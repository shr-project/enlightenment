#include "ui.h"
#ifdef HAVE_DBUS

#ifdef HAVE_NOTIFY
#include <E_Notify.h>
#endif

static DBusMessage *
_dbus_quit_cb(E_DBus_Object *obj, DBusMessage *msg)
{
   Contact_List *cl = e_dbus_object_data_get(obj);
   shotgun_disconnect(cl->account);
   return dbus_message_new_method_return(msg);
}

static DBusMessage *
_dbus_link_show_cb(E_DBus_Object *obj, DBusMessage *msg)
{
   Contact_List *cl = e_dbus_object_data_get(obj);
   const char *url;
   DBusError error;
   Image *i;
   Elm_Entry_Anchor_Info ev;

   memset(&ev, 0, sizeof(Elm_Entry_Anchor_Info));
   ev.name = "";
   chat_conv_image_hide(NULL, cl->win, &ev);
   cl->dbus_image = NULL;
   memset(&error, 0, sizeof(DBusError));
   dbus_message_get_args(msg, &error,
     's', &url,
     DBUS_TYPE_INVALID);
   while (url)
     {
        if (!url[0]) break;
        i = eina_hash_find(cl->images, url);
        if (!i) break; // not gonna let people use us as wget
        //if (!i) url = eina_stringshare_add(url);
        chat_image_add(cl, url); // update timestamp
        //i = eina_hash_find(cl->images, url);
        if (i->url)
          {
             cl->dbus_image = i;
             break;
          }
        ev.name = url;
        chat_conv_image_show(cl, NULL, &ev);
        cl->dbus_image = i;
        break;
     }
   return dbus_message_new_method_return(msg);
}

static DBusMessage *
_dbus_list_get_cb(E_DBus_Object *obj, DBusMessage *msg)
{
   Contact_List *cl = e_dbus_object_data_get(obj);
   Eina_List *l;
   Contact *c;
   DBusMessage *ret;
   DBusMessageIter iter, arr;

   ret = dbus_message_new_method_return(msg);
   dbus_message_iter_init_append(ret, &iter);
   dbus_message_iter_open_container(&iter, DBUS_TYPE_ARRAY, "s", &arr);

   EINA_LIST_FOREACH(cl->users_list, l, c)
     {
        if (c->cur && c->cur->status && c->base->subscription)
          dbus_message_iter_append_basic(&arr, DBUS_TYPE_STRING, &c->base->jid);
     }
   dbus_message_iter_close_container(&iter, &arr);
   return ret;
}

static DBusMessage *
_dbus_list_get_all_cb(E_DBus_Object *obj, DBusMessage *msg)
{
   Contact_List *cl = e_dbus_object_data_get(obj);
   Eina_List *l;
   Contact *c;
   DBusMessage *ret;
   DBusMessageIter iter, arr;

   ret = dbus_message_new_method_return(msg);
   dbus_message_iter_init_append(ret, &iter);
   dbus_message_iter_open_container(&iter, 'a', "s", &arr);

   EINA_LIST_FOREACH(cl->users_list, l, c)
     dbus_message_iter_append_basic(&arr, 's', &c->base->jid);
   dbus_message_iter_close_container(&iter, &arr);
   return ret;
}

static DBusMessage *
_dbus_contact_status_cb(E_DBus_Object *obj, DBusMessage *msg)
{
   Contact_List *cl = e_dbus_object_data_get(obj);
   DBusMessage *reply;
   Contact *c;
   char *name, *s;
   DBusError error;

   memset(&error, 0, sizeof(DBusError));
   dbus_message_get_args(msg, &error,
     's', &name,
     DBUS_TYPE_INVALID);
   if ((!name) || (!name[0])) goto error;
   s = strchr(name, '/');
   if (s) name = strndupa(name, s - name);
   c = eina_hash_find(cl->users, name);
   if (!c) goto error;
   reply = dbus_message_new_method_return(msg);
   dbus_message_append_args(reply,
     's', &c->description,
     'u', (uintptr_t*)&c->status,
     'i', (intptr_t*)&c->priority,
     DBUS_TYPE_INVALID);
   return reply;
error:
   reply = dbus_message_new_error(msg, "org.shotgun.contact.invalid", "Contact specified was invalid or not found!");
   return reply;
}

static DBusMessage *
_dbus_contact_send_cb(E_DBus_Object *obj, DBusMessage *msg)
{
   Contact_List *cl = e_dbus_object_data_get(obj);
   DBusMessageIter iter;
   DBusMessage *reply;
   Contact *c;
   char *p, *name, *s;
   Shotgun_Message_Status st;
   Eina_Bool ret = EINA_FALSE;
   DBusError error;

   memset(&error, 0, sizeof(DBusError));
   dbus_message_get_args(msg, &error,
			 's', &name,
    's', &s,
    'u', &st,
			 DBUS_TYPE_INVALID);
   reply = dbus_message_new_method_return(msg);
   dbus_message_iter_init_append(reply, &iter);
   if ((!name) || (!name[0]) || (!s)) goto error;
   p = strchr(name, '/');
   if (p) name = strndupa(name, p - name);
   c = eina_hash_find(cl->users, name);
   if (!c) goto error;
   ret = shotgun_message_send(c->base->account, contact_jid_send_get(c), s, st, c->xhtml_im);
error:
   dbus_message_iter_append_basic(&iter, DBUS_TYPE_BOOLEAN, &ret);
   return reply;
}

static DBusMessage *
_dbus_contact_send_echo_cb(E_DBus_Object *obj, DBusMessage *msg)
{
   Contact_List *cl = e_dbus_object_data_get(obj);
   DBusMessageIter iter;
   DBusMessage *reply;
   Contact *c;
   char *p, *name, *s;
   Shotgun_Message_Status st;
   Eina_Bool ret = EINA_FALSE;
   DBusError error;

   memset(&error, 0, sizeof(DBusError));
   dbus_message_get_args(msg, &error,
			 's', &name,
    's', &s,
    'u', &st,
			 DBUS_TYPE_INVALID);
   reply = dbus_message_new_method_return(msg);
   dbus_message_iter_init_append(reply, &iter);
   if ((!name) || (!name[0]) || (!s)) goto error;
   p = strchr(name, '/');
   if (p) name = strndupa(name, p - name);
   c = eina_hash_find(cl->users, name);
   if (!c) goto error;

   ret = shotgun_message_send(c->base->account, contact_jid_send_get(c), s, st, c->xhtml_im);
   if (ret)
     chat_message_insert(c, "me", s, EINA_TRUE);
error:
   dbus_message_iter_append_basic(&iter, DBUS_TYPE_BOOLEAN, &ret);
   return reply;
}

static DBusMessage *
_dbus_contact_info_cb(E_DBus_Object *obj, DBusMessage *msg)
{
   Contact_List *cl = e_dbus_object_data_get(obj);
   char *name, *s;
   const char *d;
   Contact *c;
   DBusMessage *reply;
   DBusError error;

   memset(&error, 0, sizeof(DBusError));
   dbus_message_get_args(msg, &error,
     's', &name,
     DBUS_TYPE_INVALID);
   if ((!name) || (!name[0])) goto error;
   s = strchr(name, '/');
   if (s) name = strndupa(name, s - name);
   c = eina_hash_find(cl->users, name);
   if (!c) goto error;
   reply = dbus_message_new_method_return(msg);
   if (c->cur && c->cur->photo)
     {
        size_t size = sizeof(char) * (strlen(shotgun_jid_get(cl->account)) + strlen(c->base->jid) + 6);
        s = alloca(size);
        snprintf(s, size, "%s/%s/img", shotgun_jid_get(cl->account), c->base->jid);
     }
   else s = "";
   d = contact_name_get(c);
   dbus_message_append_args(reply,
     's', &d,
     's', &s,
     'u', (uintptr_t*)&c->status,
     'i', (intptr_t*)&c->priority,
     DBUS_TYPE_INVALID);
   return reply; /* get icon name from eet file */
error:
   reply = dbus_message_new_error(msg, "org.shotgun.contact.invalid", "Contact specified was invalid or not found!");
   return reply;
}

static DBusMessage *
_dbus_contact_icon_cb(E_DBus_Object *obj, DBusMessage *msg)
{
   Contact_List *cl = e_dbus_object_data_get(obj);
   char *name, *s;
   Contact *c;
   DBusMessage *reply;
   DBusError error;

   memset(&error, 0, sizeof(DBusError));
   dbus_message_get_args(msg, &error,
     's', &name,
     DBUS_TYPE_INVALID);
   if ((!name) || (!name[0])) goto error;
   s = strchr(name, '/');
   if (s) name = strndupa(name, s - name);
   c = eina_hash_find(cl->users, name);
   if (!c) goto error;
   reply = dbus_message_new_method_return(msg);
   if (c->cur && c->cur->photo)
     {
        size_t size = sizeof(char) * (strlen(shotgun_jid_get(cl->account)) + strlen(c->base->jid) + 6);
        s = alloca(size);
        snprintf(s, size, "%s/%s/img", shotgun_jid_get(cl->account), c->base->jid);
     }
   else s = "";
   dbus_message_append_args(reply,
     's', &s,
     DBUS_TYPE_INVALID);
   return reply; /* get icon name from eet file */
error:
   reply = dbus_message_new_error(msg, "org.shotgun.contact.invalid", "Contact specified was invalid or not found!");
   return reply;
}

void
ui_dbus_signal_message_self(Contact_List *cl, const char *jid, const char *s)
{
   DBusMessage *sig;

   sig = dbus_message_new_signal(SHOTGUN_DBUS_PATH, SHOTGUN_DBUS_METHOD_BASE ".core", "new_msg_self");
   dbus_message_append_args(sig,
     's', &jid,
     's', &s,
     DBUS_TYPE_INVALID);
   e_dbus_message_send(cl->dbus, sig, NULL, -1, NULL);
   dbus_message_unref(sig);
}

void
ui_dbus_signal_message(Contact_List *cl, Contact *c, Shotgun_Event_Message *msg)
{
   DBusMessage *sig;

   sig = dbus_message_new_signal(SHOTGUN_DBUS_PATH, SHOTGUN_DBUS_METHOD_BASE ".core", "new_msg");
   dbus_message_append_args(sig,
     's', &c->base->jid,
     's', &msg->msg,
     DBUS_TYPE_INVALID);
   e_dbus_message_send(cl->dbus, sig, NULL, -1, NULL);
   dbus_message_unref(sig);
}

void
ui_dbus_signal_status(Contact *c, Shotgun_Event_Presence *pres)
{
   DBusMessage *sig;
   const char *res, *desc;

   res = strrchr(pres->jid, '/') + 1;
   desc = pres->description ?: "";
   sig = dbus_message_new_signal(SHOTGUN_DBUS_PATH, SHOTGUN_DBUS_METHOD_BASE ".core", "status");
   dbus_message_append_args(sig,
     's', &c->base->jid,
     's', &res,
     's', &desc,
     'u', &pres->status,
     'u', &pres->type,
     'i', &pres->priority,
     DBUS_TYPE_INVALID);
   e_dbus_message_send(c->list->dbus, sig, NULL, -1, NULL);
   dbus_message_unref(sig);
}

void
ui_dbus_signal_status_self(Contact_List *cl)
{
   DBusMessage *sig;
   const char *desc;
   Shotgun_User_Status st;
   int priority;

   desc = shotgun_presence_get(cl->account, &st, &priority);
   desc = desc ?: "";

   sig = dbus_message_new_signal(SHOTGUN_DBUS_PATH, SHOTGUN_DBUS_METHOD_BASE ".core", "status_self");
   dbus_message_append_args(sig,
     's', &desc,
     'u', &st,
     'i', &priority,
     DBUS_TYPE_INVALID);
   e_dbus_message_send(cl->dbus, sig, NULL, -1, NULL);
   dbus_message_unref(sig);
}

void
ui_dbus_signal_connect_state(Contact_List *cl)
{
   DBusMessage *sig;
   Eina_Bool state;

   state = (shotgun_connection_state_get(cl->account) == SHOTGUN_CONNECTION_STATE_CONNECTED);

   sig = dbus_message_new_signal(SHOTGUN_DBUS_PATH, SHOTGUN_DBUS_METHOD_BASE ".core", "connected");
   dbus_message_append_args(sig,
     'b', &state,
     DBUS_TYPE_INVALID);
   e_dbus_message_send(cl->dbus, sig, NULL, -1, NULL);
   dbus_message_unref(sig);
}

void
ui_dbus_signal_link(Contact_List *cl, const char *url, Eina_Bool self)
{
   DBusMessage *sig;

   if (self)
     sig = dbus_message_new_signal(SHOTGUN_DBUS_PATH, SHOTGUN_DBUS_METHOD_BASE ".core", "link_self");
   else
     sig = dbus_message_new_signal(SHOTGUN_DBUS_PATH, SHOTGUN_DBUS_METHOD_BASE ".core", "link");
   dbus_message_append_args(sig,
     's', &url,
     DBUS_TYPE_INVALID);
   e_dbus_message_send(cl->dbus, sig, NULL, -1, NULL);
   dbus_message_unref(sig);
}

static void
_dbus_request_name_cb(void *data __UNUSED__, DBusMessage *msg, DBusError *err __UNUSED__)
{
   DBusError error;
   unsigned int ret;

   memset(&error, 0, sizeof(DBusError));

   dbus_message_get_args(msg, &error,
			 'u', &ret,
			 DBUS_TYPE_INVALID);
   INF("RequestName: %u", ret);
}

void
ui_dbus_init(Contact_List *cl)
{
   E_DBus_Interface *iface;

   if (cl->dbus) return;

   e_dbus_init();
#ifdef HAVE_NOTIFY
   e_notification_init();
#endif
   cl->dbus = e_dbus_bus_get(DBUS_BUS_SESSION);
   e_dbus_request_name(cl->dbus, "org.shotgun", 0, _dbus_request_name_cb, NULL);
   cl->dbus_object = e_dbus_object_add(cl->dbus, "/org/shotgun/remote", cl);
   iface = e_dbus_interface_new("org.shotgun.core");
   e_dbus_object_interface_attach(cl->dbus_object, iface);
   e_dbus_interface_signal_add(iface, "new_msg", "ss");
   e_dbus_interface_signal_add(iface, "new_msg_self", "ss");
   e_dbus_interface_signal_add(iface, "status", "sssuui");
   e_dbus_interface_signal_add(iface, "status_self", "sui");
   e_dbus_interface_signal_add(iface, "link", "s");
   e_dbus_interface_signal_add(iface, "link_self", "s");
   e_dbus_interface_signal_add(iface, "connected", "b");
   e_dbus_interface_unref(iface);

   e_dbus_interface_method_add(iface, "quit", "", "", _dbus_quit_cb);

   iface = e_dbus_interface_new("org.shotgun.link");
   e_dbus_object_interface_attach(cl->dbus_object, iface);
   e_dbus_interface_unref(iface);

   e_dbus_interface_method_add(iface, "show", "s", "", _dbus_link_show_cb);

   iface = e_dbus_interface_new("org.shotgun.list");
   e_dbus_object_interface_attach(cl->dbus_object, iface);
   e_dbus_interface_unref(iface);

   e_dbus_interface_method_add(iface, "get", "", "as", _dbus_list_get_cb);
   e_dbus_interface_method_add(iface, "get_all", "", "as", _dbus_list_get_all_cb);

   iface = e_dbus_interface_new("org.shotgun.contact");
   e_dbus_object_interface_attach(cl->dbus_object, iface);
   e_dbus_interface_unref(iface);

   e_dbus_interface_method_add(iface, "status", "s", "sui", _dbus_contact_status_cb);
   e_dbus_interface_method_add(iface, "icon", "s", "s", _dbus_contact_icon_cb);
   e_dbus_interface_method_add(iface, "info", "s", "ssui", _dbus_contact_info_cb);
   e_dbus_interface_method_add(iface, "send", "ssu", "b", _dbus_contact_send_cb);
   e_dbus_interface_method_add(iface, "send_echo", "ssu", "b", _dbus_contact_send_echo_cb);
}

#ifdef HAVE_NOTIFY
void
ui_dbus_notify(Contact_List *cl, Evas_Object *img, const char *from, const char *msg)
{
   E_Notification *n;
   E_Notification_Image *i;

   if (cl->settings->disable_notify) return;

   n = e_notification_full_new("SHOTGUN!", 0, NULL, from, msg, 5000);
   if (img)
     {
        i = e_notification_image_new();
        if (e_notification_image_init(i, img))
          e_notification_hint_image_data_set(n, i);
     }
   e_notification_send(n, NULL, NULL);
   e_notification_unref(n);
}
#endif

void
ui_dbus_shutdown(Contact_List *cl)
{
   if (!cl) return;
   if (cl->dbus_object) e_dbus_object_free(cl->dbus_object);
   if (cl->dbus) e_dbus_connection_close(cl->dbus);
   cl->dbus = NULL;
   cl->dbus_object = NULL;
#ifdef HAVE_NOTIFY
   e_notification_shutdown();
#endif
   e_dbus_shutdown();
}
#endif
