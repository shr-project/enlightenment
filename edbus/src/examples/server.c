#include "EDBus.h"
#include <Ecore.h>

#define BUS "org.Enlightenment"
#define PATH "/org/enlightenment"
#define PATH_TEST_SON "/org/enlightenment/son"
#define INTERFACE "org.enlightenment.Test"

static EDBus_Connection *conn;

static EDBus_Message *
_hello(const EDBus_Service_Interface *iface, const EDBus_Message *message)
{
   EDBus_Message *reply = edbus_message_method_return_new(message);
   edbus_message_arguments_append(reply, "s", "Hello World");
   printf("Hello\n");
   return reply;
}

static EDBus_Message *
_quit(const EDBus_Service_Interface *iface, const EDBus_Message *message)
{
   printf("Quit\n");
   ecore_main_loop_quit();
   return edbus_message_method_return_new(message);
}

enum
{
   TEST_SIGNAL_ALIVE = 0,
   TEST_SIGNAL_HELLO
};

static Eina_Bool
send_signal_alive(void *data)
{
   EDBus_Service_Interface *iface = data;
   edbus_service_signal_emit(iface, TEST_SIGNAL_ALIVE);
   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
send_signal_hello(void *data)
{
   EDBus_Service_Interface *iface = data;
   edbus_service_signal_emit(iface, TEST_SIGNAL_HELLO, "Hello World");
   return ECORE_CALLBACK_RENEW;
}

static EDBus_Message *
_send_bool(const EDBus_Service_Interface *iface, const EDBus_Message *msg)
{
   EDBus_Message *reply = edbus_message_method_return_new(msg);
   Eina_Bool bool;
   if (!edbus_message_arguments_get(msg, "b", &bool))
     printf("edbus_message_arguments_get() error\n");
   edbus_message_arguments_append(reply, "b", bool);
   return reply;
}

static EDBus_Message *
_send_byte(const EDBus_Service_Interface *iface, const EDBus_Message *msg)
{
   EDBus_Message *reply = edbus_message_method_return_new(msg);
   unsigned char byte;
   if (!edbus_message_arguments_get(msg, "y", &byte))
     printf("edbus_message_arguments_get() error\n");
   edbus_message_arguments_append(reply, "y", byte);
   return reply;
}

static EDBus_Message *
_send_uint32(const EDBus_Service_Interface *iface, const EDBus_Message *msg)
{
   EDBus_Message *reply = edbus_message_method_return_new(msg);
   unsigned int uint32;
   if (!edbus_message_arguments_get(msg, "u", &uint32))
     printf("edbus_message_arguments_get() error\n");
   edbus_message_arguments_append(reply, "u", uint32);
   return reply;
}

static EDBus_Message *
_send_int32(const EDBus_Service_Interface *iface, const EDBus_Message *msg)
{
   EDBus_Message *reply = edbus_message_method_return_new(msg);
   int int32;
   if (!edbus_message_arguments_get(msg, "i", &int32))
     printf("edbus_message_arguments_get() error\n");
   edbus_message_arguments_append(reply, "i", int32);
   return reply;
}

static EDBus_Message *
_send_int16(const EDBus_Service_Interface *iface, const EDBus_Message *msg)
{
   EDBus_Message *reply = edbus_message_method_return_new(msg);
   short int int16;
   if (!edbus_message_arguments_get(msg, "n", &int16))
     printf("edbus_message_arguments_get() error\n");
   edbus_message_arguments_append(reply, "n", int16);
   return reply;
}

static EDBus_Message *
_send_double(const EDBus_Service_Interface *iface, const EDBus_Message *msg)
{
   EDBus_Message *reply = edbus_message_method_return_new(msg);
   double d;
   if (!edbus_message_arguments_get(msg, "d", &d))
     printf("edbus_message_arguments_get() error\n");
   edbus_message_arguments_append(reply, "d", d);
   return reply;
}

static EDBus_Message *
_send_string(const EDBus_Service_Interface *iface, const EDBus_Message *msg)
{
   EDBus_Message *reply = edbus_message_method_return_new(msg);
   const char *txt;
   if (!edbus_message_arguments_get(msg, "s", &txt))
     printf("edbus_message_arguments_get() error\n");
   edbus_message_arguments_append(reply, "s", txt);
   return reply;
}

static Eina_Bool
_resp_async(void *data)
{
   EDBus_Message *msg = data;
   edbus_message_arguments_append(msg, "s", "Async test ok");
   edbus_connection_send(conn, msg, NULL, NULL, -1);
   edbus_message_unref(msg);
   return ECORE_CALLBACK_CANCEL;
}

static EDBus_Message *
_async_test(const EDBus_Service_Interface *iface, const EDBus_Message *msg)
{
   EDBus_Message *reply = edbus_message_method_return_new(msg);
   printf("Received a call to AsyncTest.\n");
   printf("Response will be send in 5 seconds.\n");
   ecore_timer_add(5, _resp_async, reply);
   return NULL;
}

static const EDBus_Signal signals[] = {
   [TEST_SIGNAL_ALIVE] = {"Alive", NULL, 0},
   [TEST_SIGNAL_HELLO] = {"Hello", EDBUS_ARGS({ "s", "message" }), 0},
   { }
};

static const EDBus_Method methods[] = {
      {
        "Hello", NULL, EDBUS_ARGS({"s", "message"}),
        _hello
      },
      {
        "Quit", NULL, NULL,
        _quit, EDBUS_METHOD_FLAG_DEPRECATED
      },
      { "SendBool", EDBUS_ARGS({"b", "bool"}), EDBUS_ARGS({"b", "bool"}),
        _send_bool
      },
      { "SendByte", EDBUS_ARGS({"y", "byte"}), EDBUS_ARGS({"y", "byte"}),
        _send_byte
      },
      { "SendUint32", EDBUS_ARGS({"u", "uint32"}), EDBUS_ARGS({"u", "uint32"}),
        _send_uint32
      },
      { "SendInt32", EDBUS_ARGS({"i", "int32"}), EDBUS_ARGS({"i", "int32"}),
        _send_int32
      },
      { "SendInt16", EDBUS_ARGS({"n", "int16"}), EDBUS_ARGS({"n", "int16"}),
        _send_int16
      },
      { "SendDouble", EDBUS_ARGS({"d", "double"}), EDBUS_ARGS({"d", "double"}),
        _send_double
      },
      { "SendString", EDBUS_ARGS({"s", "string"}), EDBUS_ARGS({"s", "string"}),
        _send_string
      },
      { "AsyncTest", NULL, EDBUS_ARGS({"s", "text"}),
        _async_test
      },
      { }
};

static const EDBus_Service_Interface_Desc iface_desc = {
   INTERFACE, methods, signals
};

static void
on_name_request(void *data, const EDBus_Message *msg, EDBus_Pending *pending)
{
   EDBus_Service_Interface *iface;
   unsigned int reply;

   iface = data;
   if (edbus_message_error_get(msg, NULL, NULL))
     {
        printf("error on on_name_request\n");
        return;
     }

   if (!edbus_message_arguments_get(msg, "u", &reply))
    {
       printf("error geting arguments on on_name_request\n");
       return;
    }

   if (reply != EDBUS_NAME_REQUEST_REPLY_PRIMARY_OWNER)
     {
        printf("error name already in use\n");
        return;
     }

   ecore_timer_add(5, send_signal_alive, iface);
   ecore_timer_add(6, send_signal_hello, iface);
}

int
main(void)
{
   EDBus_Service_Interface *iface;

   ecore_init();
   edbus_init();

   conn = edbus_connection_get(EDBUS_CONNECTION_TYPE_SESSION);

   iface = edbus_service_interface_register(conn, PATH, &iface_desc);
   edbus_name_request(conn, BUS, EDBUS_NAME_REQUEST_FLAG_DO_NOT_QUEUE,
                      on_name_request, iface);

   edbus_service_interface_register(conn, PATH_TEST_SON, &iface_desc);

   ecore_main_loop_begin();

   edbus_connection_unref(conn);

   edbus_shutdown();
   ecore_shutdown();
   return 0;
}
