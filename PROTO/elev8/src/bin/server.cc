#include <unistd.h>
#include "server.h"

#define SHUTDOWN_COMMAND 's'
#define RUN_COMMAND 'r'

#define CURRENT_DIR_SIZE 64
#define SCRIPT_NAME_SIZE 32

using namespace v8;

struct Server_Command {
   char command;
   char current_dir[CURRENT_DIR_SIZE];
   char script[SCRIPT_NAME_SIZE];
   int argc;
   char argv[];
};

struct Server_Command_Wrapper {
   struct Server_Command *command;
   int command_size;
};

Ecore_Con_Server *server;

static char**
split_argv(int argc, char *argv)
{
   int i = 0;

   char *save_ptr;
   char **result = (char**)malloc(sizeof(char*) * argc);
   if (!result)
     goto fail;

   char *arg;
   arg = strtok_r(argv, "\n", &save_ptr);
   while (arg != NULL)
     {
        if ((result[i++] = strdup(arg)) == NULL)
          {
             ERR("Couldn't allocate memory");
             break;
          }
        arg = strtok_r(NULL, "\n", &save_ptr);
     }

   return result;

fail:
  ERR("Couldn't allocate memory");
  return NULL;
}

static int
concat_argv(char **result, int argc, char **argv)
{
   char *concat;

   int argv_size = 0;
   for (int i = 0; i < argc; i++)
     argv_size += strlen(argv[i]);

   *result = concat = (char*)malloc(argv_size + argc + 1);
   if (!*result)
     goto fail;

   for (int i = 0; i < argc; i++)
     {
        concat = (char*)mempcpy(concat, argv[i], strlen(argv[i]));
        *concat++ = '\n';
     }
   *concat = '\0';

   return argv_size + argc + 1;

fail:
   ERR("Couldn't allocate memory");
   return -1;
}

static void
spawn_and_run(struct Server_Command *command)
{
   int child_pid = fork();
   char **splitted_argv = NULL;

   if (child_pid < 0)
     {
        ERR("Couldn't create child process");
        return;
     }
   else if (child_pid > 0)
     return;

   // From here is the child
   ecore_fork_reset();
   ecore_con_server_del(server); // So the child won't be listening on socket

   if (chdir(command->current_dir) == -1)
     {
        ERR("Couldn't change to current dir");
        goto end;
     }

   splitted_argv = split_argv(command->argc, command->argv);
   execute_elev8_script(command->script, command->argc, splitted_argv);

end:
   for (int i = 0; i < command->argc && splitted_argv != NULL; i++)
     free(splitted_argv[i]);
   free(splitted_argv);
}

static Eina_Bool
_cb_conn_data(void *, int, Ecore_Con_Event_Client_Data *ev)
{
   Server_Command *command = static_cast<Server_Command *>(ev->data);

   if (command->command == SHUTDOWN_COMMAND)
     ecore_main_loop_quit();
   else
     spawn_and_run(command);

   return ECORE_CALLBACK_RENEW;
}

void
server_start()
{
   if (!(server = ecore_con_server_add(ECORE_CON_LOCAL_SYSTEM, ELEV8_SOCK_PATH,
                                       ELEV8_SOCK_PORT, NULL)))
     {
        ERR("Couldn't create server control socket, check permissions for %s:%d.",
            ELEV8_SOCK_PATH, ELEV8_SOCK_PORT);
        exit(-1);
     }

   ecore_event_handler_add(ECORE_CON_EVENT_CLIENT_DATA,
                          (Ecore_Event_Handler_Cb)_cb_conn_data, NULL);

   ecore_con_server_client_limit_set(server, ELEV8_LISTEN_BACKLOG, 0);

   load_elev8_modules();
}

static Eina_Bool
_cb_server_add(void *data, int, Ecore_Con_Event_Server_Add *ev)
{
   Server_Command_Wrapper *scw = static_cast<Server_Command_Wrapper *>(data);

   ecore_con_server_send(ev->server, scw->command, scw->command_size);
   ecore_con_server_flush(ev->server);

   ecore_main_loop_quit();

   free(scw->command);
   free(scw);

   return ECORE_CALLBACK_DONE;
}

void
server_shutdown()
{
   Server_Command *command;
   Server_Command_Wrapper *scw;

   command = (Server_Command*)malloc(sizeof(*command));
   if (!command)
     goto fail;

   command->command = SHUTDOWN_COMMAND;

   scw = (Server_Command_Wrapper*)malloc(sizeof(*scw));
   if (!scw)
     goto fail_scw;

   scw->command = command;
   scw->command_size = sizeof(*command);

   ecore_event_handler_add(ECORE_CON_EVENT_SERVER_ADD,
                       (Ecore_Event_Handler_Cb)_cb_server_add, scw);

   if (!(server = ecore_con_server_connect(ECORE_CON_LOCAL_SYSTEM,
                                           ELEV8_SOCK_PATH, ELEV8_SOCK_PORT,
                                           NULL)))
     {
        ERR("Unable to connect to server\n");
        ecore_main_loop_quit();
     }

   return;

fail_scw:
   free(command);
fail:
   ERR("Couldn't allocate memory");
}

void
server_spawn(int script_arg, int argc, char *argv[])
{
   int argv_size;
   char *argv_concat = NULL;
   char *current_dir = NULL;
   Server_Command *command = NULL;
   Server_Command_Wrapper *scw = NULL;

   argv_size = concat_argv(&argv_concat, argc, argv);
   if (argv_size == -1)
     return;

   command = (Server_Command*)malloc(sizeof(*command) + argv_size);
   if (!command)
     goto fail_malloc;

   if ((strlen(argv[script_arg]) + 1) > SCRIPT_NAME_SIZE)
     {
        ERR("App name is too big. It must have up to %d characters",
            SCRIPT_NAME_SIZE - 1);
        goto fail;
     }

   current_dir = get_current_dir_name();
   if ((strlen(current_dir) + 1) > CURRENT_DIR_SIZE)
     {
        ERR("Current dir path is too big. It must have up to %d characters",
            CURRENT_DIR_SIZE - 1);
        goto fail;
     }

   memset(command, 0, sizeof(*command) + argv_size);
   command->command = RUN_COMMAND;
   strncpy(command->script, argv[script_arg], SCRIPT_NAME_SIZE);
   strncpy(command->current_dir, get_current_dir_name(), CURRENT_DIR_SIZE);
   command->argc = argc;
   memcpy(command->argv, argv_concat, argv_size);

   scw = (Server_Command_Wrapper*)malloc(sizeof(*scw));
   if (!scw)
     goto fail_malloc;

   scw->command = command;
   scw->command_size = sizeof(*command) + argv_size;

   ecore_event_handler_add(ECORE_CON_EVENT_SERVER_ADD,
                       (Ecore_Event_Handler_Cb)_cb_server_add, scw);

   if (!ecore_con_server_connect(ECORE_CON_LOCAL_SYSTEM,
                                           ELEV8_SOCK_PATH, ELEV8_SOCK_PORT,
                                           NULL))
     {
       WRN("Unable to connect to server. Running locally.\n");
       load_elev8_modules();
       execute_elev8_script(argv[script_arg], argc, argv);
     }

   free(argv_concat);
   return;

fail_malloc:
   ERR("Couldn't allocate memory");
fail:
   free(command);
   free(argv_concat);
   free(current_dir);
   exit(-1);
}
