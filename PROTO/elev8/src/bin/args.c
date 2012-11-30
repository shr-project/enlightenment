#include <Ecore.h>
#include <Ecore_Getopt.h>
#include "args.h"

int log_domain;

#define ARGS_ERR(...) EINA_LOG_DOM_ERR(log_domain, __VA_ARGS__)

static const Ecore_Getopt optdesc = {
  "",
  "\n\telev8 input_file.js\n\te.g. elev8 ../../data/javascript/button.js\n",
  "0.0",
  "(C) 2012 Enlightenment",
  "GPLv3",
  "EFL Elev8 JavaScript interpreter.",
  0,
  {
    ECORE_GETOPT_STORE_DEF_BOOL(0, "debug", "enable debuggind mode", EINA_TRUE),
    ECORE_GETOPT_STORE_DEF_BOOL(0, "server", "enable server mode", EINA_TRUE),
    ECORE_GETOPT_STORE_DEF_BOOL(0, "shutdown", "shutdown elev8 server.", EINA_TRUE),
    ECORE_GETOPT_STORE_DEF_BOOL(0, "connect", "run app spawing a elev8 server.", EINA_TRUE),
    ECORE_GETOPT_HELP('h', "help"),
    ECORE_GETOPT_LICENSE('l', "license"),
    ECORE_GETOPT_VERSION('v', "version"),
    ECORE_GETOPT_SENTINEL
  }
};

int elev8_parse_argv(elev8_args *args, int argc, char **argv)
{
   int status;

   Ecore_Getopt_Value values[] = {
     ECORE_GETOPT_VALUE_BOOL(args->debug),
     ECORE_GETOPT_VALUE_BOOL(args->server),
     ECORE_GETOPT_VALUE_BOOL(args->shutdown),
     ECORE_GETOPT_VALUE_BOOL(args->connect),
     ECORE_GETOPT_VALUE_BOOL(args->quit),
     ECORE_GETOPT_VALUE_BOOL(args->quit),
     ECORE_GETOPT_VALUE_BOOL(args->quit),
     ECORE_GETOPT_VALUE_NONE
   };

   status = ecore_getopt_parse(&optdesc, values, argc, argv);

   if (status < 0)
     ARGS_ERR("Elev8 argument parsing failed\n");

   return status;
}
