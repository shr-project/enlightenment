#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "private.h"
#include <EDBus.h>

#define DBUS_NAME "org.enlightenment.enjoy"
#define DBUS_IFACE "org.enlightenment.enjoy.Control"
#define DBUS_PATH "/org/enlightenment/enjoy/Control"

static EDBus_Connection *conn;
static EDBus_Service_Interface *control;

static EDBus_Message *
_cb_dbus_quit(const EDBus_Service_Interface *iface __UNUSED__, const EDBus_Message *msg)
{
   enjoy_quit();
   return edbus_message_method_return_new(msg);
}

static EDBus_Message *
_cb_dbus_version(const EDBus_Service_Interface *iface __UNUSED__, const EDBus_Message *msg)
{
   EDBus_Message *reply = edbus_message_method_return_new(msg);
   uint16_t aj = VMAJ, in = VMIN, ic = VMIC;
   edbus_message_arguments_set(reply, "qqq", aj, in, ic);
   return reply;
}

/* Avoid duplicating MPRIS -- see src/plugins/mpris */
static const EDBus_Method control_methods[] = {
   { "Quit", NULL, NULL, _cb_dbus_quit, 0 },
   {
    "Version", NULL, EDBUS_ARGS({"q", ""}, {"q", ""}, {"q", ""}),
    _cb_dbus_version, 0
   },
   /* TODO: DB management */
   { }
};

static const EDBus_Service_Interface_Desc desc = {
   DBUS_IFACE, control_methods
};

static void
_cb_dbus_request_name(void *data __UNUSED__, const EDBus_Message *msg, EDBus_Pending *pending__UNUSED__)
{
   const char *error_name, *error_txt;
   unsigned flag;

   if (edbus_message_error_get(msg, &error_name, &error_txt))
     {
        ERR("Error %s %s", error_name, error_txt);
        goto end;
     }

   if (!edbus_message_arguments_get(msg, "u", &flag))
     {
        ERR("Error getting arguments.");
        goto end;
     }

   if (flag != EDBUS_NAME_REQUEST_REPLY_PRIMARY_OWNER)
     {
        ERR("Bus name in use by another application.");
        goto end;
     }

   INF("Got DBus name - unique instance running.");
   edbus_service_interface_register(conn, DBUS_PATH, &desc);

   /* will run after other events run, in the main loop */
   ecore_event_add(ENJOY_EVENT_STARTED, NULL, NULL, NULL);
   return;

end:
   ecore_main_loop_quit();
}

Eina_Bool
enjoy_dbus_init(void)
{
   edbus_init();
   conn = edbus_connection_get(EDBUS_CONNECTION_TYPE_SESSION);
   if (!conn)
     {
        ERR("Could not get DBus session bus");
        return EINA_FALSE;
     }

   edbus_name_request(conn, DBUS_NAME, EDBUS_NAME_REQUEST_FLAG_DO_NOT_QUEUE,
                      _cb_dbus_request_name, NULL);
   return EINA_TRUE;
}

void
enjoy_dbus_shutdown(void)
{
   if (control)
     edbus_service_interface_unregister(control);
   if (conn)
     edbus_connection_unref(conn);
   edbus_shutdown();
   conn = NULL;
   control = NULL;
}
