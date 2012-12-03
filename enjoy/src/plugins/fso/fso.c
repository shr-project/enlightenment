#include <Eina.h>
#include <EDBus.h>
#include <Ecore.h>
#include "plugin.h"

static int _fso_log_domain = -1;

#ifdef CRITICAL
#undef CRITICAL
#endif
#ifdef ERR
#undef ERR
#endif
#ifdef WRN
#undef WRN
#endif
#ifdef INF
#undef INF
#endif
#ifdef DBG
#undef DBG
#endif

#define CRITICAL(...) EINA_LOG_DOM_CRIT(_fso_log_domain, __VA_ARGS__)
#define ERR(...)      EINA_LOG_DOM_ERR(_fso_log_domain, __VA_ARGS__)
#define WRN(...)      EINA_LOG_DOM_WARN(_fso_log_domain, __VA_ARGS__)
#define INF(...)      EINA_LOG_DOM_INFO(_fso_log_domain, __VA_ARGS__)
#define DBG(...)      EINA_LOG_DOM_DBG(_fso_log_domain, __VA_ARGS__)

#define FSO_OUSAGED_SERVICE "org.freesmartphone.ousaged"
#define FSO_OUSAGED_OBJECT_PATH "/org/freesmartphone/Usage"
#define FSO_OUSAGED_INTERFACE "org.freesmartphone.Usage"

static EDBus_Connection *conn = NULL;
static EDBus_Proxy *proxy = NULL;

typedef struct _FSO_Cb_Data
{
   void (*func)(void *data, Eina_Bool error);
   void *data;
} FSO_Cb_Data;

static void
fso_request_resource_cb(void *data, const EDBus_Message *msg, EDBus_Pending *pending)
{
   FSO_Cb_Data *d = data;
   Eina_Bool e = EINA_FALSE;
   const char *error_name, *error_txt;

   DBG("Request sent to fsousaged to enable resource.");

   if (edbus_message_error_get(msg, &error_name, &error_txt))
     {
        ERR("Error requesting FSO resource: %s - %s", error_name, error_txt);
        e = EINA_TRUE;
     }

   if ((d) && (d->func))
     d->func(d->data, e);
   free(d);
}

static void
fso_release_resource_cb(void *data, const EDBus_Message *msg, EDBus_Pending *pending)
{
   FSO_Cb_Data *d = data;
   Eina_Bool e = EINA_FALSE;
   const char *error_name, *error_txt;

   DBG("Request sent to fsousaged to disable resource.");

   if (edbus_message_error_get(msg, &error_name, &error_txt))
     {
        ERR("Error releasing FSO resource: %s - %s", error_name, error_txt);
        e = EINA_TRUE;
     }

   if ((d) && (d->func))
     d->func(d->data, e);
   free(d);
}

static void
fso_request_resource(const char *resource, void (*func)(void *data, Eina_Bool error), const void *data)
{
   FSO_Cb_Data *d = NULL;
   EINA_SAFETY_ON_NULL_RETURN(proxy);

   if (func)
     {
        d = malloc(sizeof(FSO_Cb_Data));
        if (d)
          {
             d->func = func;
             d->data = (void *)data;
          }
     }
   edbus_proxy_call(proxy, "RequestResource", fso_request_resource_cb, d, -1,
                    "s", resource);
}


static void
fso_release_resource(const char *resource, void (*func)(void *data, Eina_Bool error), const void *data)
{
   FSO_Cb_Data *d = NULL;
   EINA_SAFETY_ON_NULL_RETURN(proxy);

   if (func)
     {
        d = malloc(sizeof(FSO_Cb_Data));
        if (d)
          {
             d->func = func;
             d->data = (void *)data;
          }
     }
   edbus_proxy_call(proxy, "RequestResource", fso_release_resource_cb, d, -1,
                    "s", resource);
}

static Eina_Bool
fso_enable(Enjoy_Plugin *p __UNUSED__)
{
   fso_request_resource("CPU", NULL, NULL);
   return EINA_TRUE;
}

static void
_cb_fso_release_resource_done(void *data __UNUSED__, Eina_Bool error __UNUSED__)
{
   enjoy_quit_thaw();
}

static Eina_Bool
fso_disable(Enjoy_Plugin *p __UNUSED__)
{
   enjoy_quit_freeze();
   fso_release_resource("CPU", _cb_fso_release_resource_done, NULL);
   return EINA_TRUE;
}

static const Enjoy_Plugin_Api api = {
  ENJOY_PLUGIN_API_VERSION,
  fso_enable,
  fso_disable
};

static Eina_Bool
fso_init(void)
{
   EDBus_Object *obj;
   if (_fso_log_domain < 0)
     {
        _fso_log_domain = eina_log_domain_register
          ("enjoy-fso", EINA_COLOR_LIGHTCYAN);
        if (_fso_log_domain < 0)
          {
             EINA_LOG_CRIT("Could not register log domain 'enjoy-fso'");
             return EINA_FALSE;
          }
     }

   if (!ENJOY_ABI_CHECK())
     {
        ERR("ABI versions differ: enjoy=%u, fso=%u",
            enjoy_abi_version(), ENJOY_ABI_VERSION);
        goto error;
     }

   if (conn) return EINA_TRUE;

   edbus_init();
   conn = edbus_connection_get(EDBUS_CONNECTION_TYPE_SYSTEM);
   if (!conn)
     {
        ERR("Could not get DBus session bus");
        goto error;
     }
   obj = edbus_object_get(conn, FSO_OUSAGED_SERVICE, FSO_OUSAGED_OBJECT_PATH);
   proxy = edbus_proxy_get(obj, FSO_OUSAGED_INTERFACE);

   enjoy_plugin_register("sys/fso", &api, ENJOY_PLUGIN_PRIORITY_NORMAL);

   return EINA_TRUE;

 error:
   eina_log_domain_unregister(_fso_log_domain);
   _fso_log_domain = -1;
   return EINA_FALSE;
}

static void
fso_shutdown(void)
{
   if (!conn) return;

   edbus_object_unref(edbus_proxy_object_get(proxy));
   proxy = NULL;
   edbus_connection_unref(conn);
   edbus_shutdown();
   conn = NULL;
   if (_fso_log_domain >= 0)
     {
        eina_log_domain_unregister(_fso_log_domain);
        _fso_log_domain = -1;
     }
}

EINA_MODULE_INIT(fso_init);
EINA_MODULE_SHUTDOWN(fso_shutdown);
