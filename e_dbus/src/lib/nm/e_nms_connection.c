#include "E_Nm.h"
#include "e_nm_private.h"
#include "e_dbus_private.h"
#include <string.h>

static void
cb_updated(void *data, DBusMessage *msg)
{
  E_NMS_Connection_Internal *conn;
  Ecore_Hash                *settings;
  if (!msg || !data) return;

  conn = data;
  settings = parse_settings(msg);

  if (conn->updated)
    conn->updated(&(conn->conn), settings);
}

static void
cb_settings(void *data, DBusMessage *msg, DBusError *err)
{
  Reply_Data *d;
  Ecore_Hash *settings;

  d = data;
  if (dbus_error_is_set(err))
  {
    E_DBUS_LOG_ERR("%s - %s", err->name, err->message);
    d->cb_func(d->data, NULL);
    free(d);
    return;
  }

  settings = parse_settings(msg);
  d->cb_func(d->data, settings);
  free(d);
}

static void
cb_secrets(void *data, DBusMessage *msg, DBusError *err)
{
  Reply_Data *d;
  Ecore_Hash *secrets;

  d = data;
  if (dbus_error_is_set(err))
  {
    E_DBUS_LOG_ERR("%s - %s", err->name, err->message);
    d->cb_func(d->data, NULL);
    free(d);
    return;
  }

  secrets = parse_settings(msg);
  d->cb_func(d->data, secrets);
  free(d);
}

EAPI E_NMS_Connection *
e_nms_connection_get(E_NMS *nms, const char *service_name, const char *connection)
{
  E_NMS_Internal            *nmsi;
  E_NMS_Connection_Internal *conn;

  nmsi = (E_NMS_Internal *)nms;
  conn = calloc(1, sizeof(E_NMS_Connection_Internal));
  conn->nmi = nmsi->nmi;
  conn->conn.path = strdup(connection);
  conn->conn.service_name = strdup(service_name);
  conn->handlers = ecore_list_new();
  ecore_list_append(conn->handlers, e_nms_connection_signal_handler_add(nmsi->nmi->conn, service_name, connection, "Updated", cb_updated, conn));

  return &conn->conn;
}

EAPI void
e_nms_connection_free(E_NMS_Connection *connection)
{
  E_NMS_Connection_Internal *conn;

  if (!connection) return;
  conn = (E_NMS_Connection_Internal *)connection;

  if (conn->conn.service_name) free(conn->conn.service_name);
  if (conn->conn.path) free(conn->conn.path);
  if (conn->handlers)
  {
    E_DBus_Signal_Handler *sh;

    while ((sh = ecore_list_first_remove(conn->handlers)))
      e_dbus_signal_handler_del(conn->nmi->conn, sh);
    ecore_list_destroy(conn->handlers);
  }
  free(conn);
}

EAPI void
e_nms_connection_dump(E_NMS_Connection *conn)
{
  if (!conn) return;

  E_DBUS_LOG_INFO("E_NMS_Connection:");
  E_DBUS_LOG_INFO("service_name: %s", conn->service_name);
  E_DBUS_LOG_INFO("path        : %s", conn->path);
  E_DBUS_LOG_INFO("");
}

EAPI int
e_nms_connection_get_settings(E_NMS_Connection *connection, int (*cb_func)(void *data, Eina_Hash *settings), void *data)
{
  DBusMessage *msg;
  Reply_Data   *d;
  E_NMS_Connection_Internal *conn;
  int ret;

  conn = (E_NMS_Connection_Internal *)connection;
  d = calloc(1, sizeof(Reply_Data));
  d->object = conn;
  d->cb_func = OBJECT_CB(cb_func);
  d->data = data;

  msg = e_nms_connection_call_new(conn->conn.service_name, conn->conn.path, "GetSettings");

  ret = e_dbus_message_send(conn->nmi->conn, msg, cb_settings, -1, d) ? 1 : 0;
  dbus_message_unref(msg);
  return ret;
}

EAPI int
e_nms_connection_secrets_get_secrets(E_NMS_Connection *connection, const char *setting_name, Ecore_List *hints, int request_new, int (*cb_func)(void *data, Eina_Hash *secrets), void *data)
{
  DBusMessage      *msg;
  DBusMessageIter   iter, a_iter;
  Reply_Data       *d;
  E_NMS_Connection_Internal *conn;
  int ret;

  conn = (E_NMS_Connection_Internal *)connection;
  d = calloc(1, sizeof(Reply_Data));
  d->object = conn;
  d->cb_func = OBJECT_CB(cb_func);
  d->data = data;

  msg = e_nms_connection_secrets_call_new(conn->conn.service_name, conn->conn.path, "GetSecrets");
  dbus_message_iter_init_append(msg, &iter);
  dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING, &setting_name);
  dbus_message_iter_open_container(&iter, DBUS_TYPE_ARRAY, "s", &a_iter);
  if (hints)
  {
    const char *hint;
    ecore_list_first_goto(hints);
    while ((hint = ecore_list_next(hints)))
      dbus_message_iter_append_basic(&a_iter, DBUS_TYPE_STRING, &hint);
  }
  dbus_message_iter_close_container(&iter, &a_iter);
  dbus_message_iter_append_basic(&iter, DBUS_TYPE_BOOLEAN, &request_new);

  ret = e_dbus_message_send(conn->nmi->conn, msg, cb_secrets, -1, d) ? 1 : 0;
  dbus_message_unref(msg);
  return ret;
}

EAPI void
e_nms_connection_callback_updated_set(E_NMS_Connection *connection, int (*cb_func)(E_NMS_Connection *conn, Ecore_Hash *settings))
{
  E_NMS_Connection_Internal *conn;

  conn = (E_NMS_Connection_Internal*)connection;
  conn->updated = cb_func;
}

