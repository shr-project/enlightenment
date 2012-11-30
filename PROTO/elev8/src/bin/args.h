#ifndef ARGS_H
#define ARGS_H
#include <stdio.h>
#include <Eina.h>
#endif

#ifdef __cplusplus

extern "C" {
#endif
  typedef struct {
     Eina_Bool debug;
     Eina_Bool server;
     Eina_Bool shutdown;
     Eina_Bool connect;
     Eina_Bool quit;
  } elev8_args;

  extern int elev8_parse_argv(elev8_args *args, int argc, char **argv);
#ifdef __cplusplus
};
#endif

