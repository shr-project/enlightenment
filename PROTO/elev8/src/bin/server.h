#ifndef __ELEV8_SERVER_H_
#define __ELEV8_SERVER_H_

#include <v8.h>
#include <Ecore.h>
#include <Ecore_Con.h>
#include <elev8_common.h>
#include "main.h"

#define ELEV8_SOCK_PATH "elev8-serversockpath"
#define ELEV8_SOCK_PORT 6523
#define ELEV8_LISTEN_BACKLOG 30

using namespace v8;

void server_shutdown();
void server_start();
void server_spawn(const int script_arg, int argc, char *argv[]);

#endif
