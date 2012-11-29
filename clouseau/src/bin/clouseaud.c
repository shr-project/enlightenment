#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <fcntl.h>
#include <Ecore_Con_Eet.h>

#include "clouseau_private.h"

#define RUNNING_DIR  "/tmp"
#define LOCK_FILE ".clouseaud.lock"
#define LOG_FILE  "clouseaud.log"

static Eina_List *gui = NULL; /* List of app_info_st for gui clients */
static Eina_List *app = NULL; /* List of app_info_st for app clients */
static Ecore_Con_Eet *eet_svr = NULL;

/* For Debug */
char msg_buf[MAX_LINE+1];

struct _tree_info_st
{
   void *app;    /* app ptr to identify where the data came from */
   void *data;   /* Tree data */
};
typedef struct _tree_info_st tree_info_st;

static void
log_message(char *filename, char *mode, char *message)
{
   FILE *logfile;
   logfile=fopen(filename, mode);
   if(!logfile) return;
   fprintf(logfile,"%s\n",message);
   fclose(logfile);
}

static void
_daemon_cleanup(void)
{  /*  Free strings */
   app_info_st *p;
   time_t currentTime;

   time (&currentTime);
   sprintf(msg_buf,"\n\n%sClients connected to this server when exiting: %d\n"
         , ctime(&currentTime), eina_list_count(app) + eina_list_count(gui));
   log_message(LOG_FILE, "a", msg_buf);

   EINA_LIST_FREE(gui, p)
     {
        if(p->file)
          free(p->file);

        if (p->ptr)
          free((void *) (uint32_t) p->ptr);

        free(p->name);
        free(p);
     }

   EINA_LIST_FREE(app, p)
     {
        if(p->file)
          free(p->file);

        if (p->ptr)
          free((void *) (uint32_t) p->ptr);

        free(p->name);
        free(p);
     }

   gui = app = NULL;
   eet_svr = NULL;

   clouseau_data_shutdown();
   ecore_con_shutdown();
   ecore_shutdown();
   eina_shutdown();
}

void daemonize(void)
{
   int i,lfp;
   char str[10];
   time_t currentTime;

   if(getppid()==1) return; /* already a daemon */
   i=fork();
   if (i<0) exit(1); /* fork error */
   if (i>0) exit(0); /* parent exits */

   time (&currentTime);

   /* child (daemon) continues */
   setsid(); /* obtain a new process group */
   for (i=getdtablesize();i>=0;--i) close(i); /* close all descriptors */
   i=open("/dev/null",O_RDWR);
   if (dup(i) == -1) return; /* handle standart I/O */
   if (dup(i) == -1) return; /* handle standart I/O */
   umask(027); /* set newly created file permissions */
   if (chdir(RUNNING_DIR) == -1) return; /* change running directory */
   lfp=open(LOCK_FILE,O_RDWR|O_CREAT,0640);
   if (lfp<0) exit(1); /* can not open */
   if (lockf(lfp,F_TLOCK,0)<0) exit(0); /* can not lock */
   /* first instance continues */
   sprintf(str,"%d\n",getpid());
   if (write(lfp,str,strlen(str)) == -1) return; /* record pid to lockfile */
   signal(SIGCHLD,SIG_IGN); /* ignore child */
   signal(SIGTSTP,SIG_IGN); /* ignore tty signals */
   signal(SIGTTOU,SIG_IGN);
   signal(SIGTTIN,SIG_IGN);

   log_message(LOG_FILE, "w", "Daemon Started");
   log_message(LOG_FILE, "a", ctime(&currentTime));
}

/* START - Ecore communication callbacks */
static int
_client_ptr_cmp(const void *d1, const void *d2)
{
   return (((app_info_st *) d1)->ptr - ((unsigned long long) (uintptr_t) d2));
}

static Eina_List *
_add_client(Eina_List *clients, connect_st *t, void *client, const char *type)
{
   sprintf(msg_buf, "\n<%s> msg from <%p>", __func__, client);
   log_message(LOG_FILE, "a", msg_buf);

   if(!eina_list_search_unsorted(clients, _client_ptr_cmp, client))
     {
        app_info_st *st = calloc(1, sizeof(app_info_st));
        st->name = strdup(t->name);
        st->pid = t->pid;
        st->ptr = (unsigned long long) (uintptr_t) client;
        sprintf(msg_buf, "\tAdded %s client <%p>", type, client);
        log_message(LOG_FILE, "a", msg_buf);
        return eina_list_append(clients, st);
     }

   return clients;
}

static Eina_List *
_remove_client(Eina_List *clients, void *client, const char *type)
{
   app_info_st *p = eina_list_search_unsorted(clients, _client_ptr_cmp, client);
   sprintf(msg_buf, "\n<%s> msg from <%p>", __func__, client);
   log_message(LOG_FILE, "a", msg_buf);

   if (p)
     {
        free(p->name);
        free(p);
        sprintf(msg_buf, "\tRemoved %s client <%p>", type, client);
        log_message(LOG_FILE, "a", msg_buf);
        return eina_list_remove(clients, p);
     }

   return clients;
}

Eina_Bool
_add(EINA_UNUSED void *data, Ecore_Con_Reply *reply,
      EINA_UNUSED Ecore_Con_Client *conn)
{
/* TODO:   ecore_ipc_client_data_size_max_set(ev->client, -1); */
   sprintf(msg_buf, "\n<%s> msg from <%p>", __func__, reply);
   log_message(LOG_FILE, "a", msg_buf);

   return ECORE_CALLBACK_RENEW;
}

Eina_Bool
_del(EINA_UNUSED void *data, Ecore_Con_Reply *reply,
      EINA_UNUSED Ecore_Con_Client *conn)
{
   sprintf(msg_buf, "\n<%s> msg from <%p>", __func__, reply);
   log_message(LOG_FILE, "a", msg_buf);

   /* Now we need to find if its an APP or GUI client */
   app_info_st *i = eina_list_search_unsorted(gui, _client_ptr_cmp, reply);
   if (i)  /* Only need to remove GUI client from list */
     gui = _remove_client(gui, reply, "GUI");

   i = eina_list_search_unsorted(app, _client_ptr_cmp, reply);
   if (i)
     {  /* Notify all GUI clients to remove this APP */
        app_closed_st t = { (unsigned long long) (uintptr_t) reply };
        Eina_List *l;
        EINA_LIST_FOREACH(gui, l, i)
          {
             sprintf(msg_buf, "\t<%p> Sending APP_CLOSED to <%p>",
                   reply, (void *) (uint32_t) i->ptr);
             log_message(LOG_FILE, "a", msg_buf);
             ecore_con_eet_send((void *) (uintptr_t) i->ptr,
                   CLOUSEAU_APP_CLOSED_STR, &t);
          }

        app = _remove_client(app, reply, "APP");
     }

   if (!(gui || app))
     {  /* Trigger cleanup and exit when all clients disconneced */
        /* ecore_con_eet_server_free(eet_svr); why this causes Segfault? */
        ecore_con_server_del(data);
        ecore_main_loop_quit();
     }

   return ECORE_CALLBACK_RENEW;
}

void
_gui_client_connect_cb(EINA_UNUSED void *data, Ecore_Con_Reply *reply,
      EINA_UNUSED const char *protocol_name, void *value)
{  /* Register GUI, then notify about all APP */
   app_info_st *st;
   Eina_List *l;
   connect_st *t = value;
   sprintf(msg_buf, "\n<%s> msg from <%p>", __func__, reply);
   log_message(LOG_FILE, "a", msg_buf);

   gui = _add_client(gui, t, reply, "GUI");

   /* Add all registered apps to newly open GUI */
   EINA_LIST_FOREACH(app, l, st)
     {
        sprintf(msg_buf, "\t<%p> Sending APP_ADD to <%p>",
              (void *) (uint32_t) st->ptr, reply);
        log_message(LOG_FILE, "a", msg_buf);
        ecore_con_eet_send(reply, CLOUSEAU_APP_ADD_STR, st);
     }
}

void
_app_client_connect_cb(EINA_UNUSED void *data, Ecore_Con_Reply *reply,
      EINA_UNUSED const char *protocol_name, void *value)
{  /* Register APP then notify GUI about it */
   app_info_st *st;
   Eina_List *l;
   connect_st *t = value;
   app_info_st m = { t->pid, (char *) t->name, NULL,
        (unsigned long long) (uintptr_t) reply, NULL, 0 };

   sprintf(msg_buf, "\n<%s> msg from <%p>", __func__, reply);
   log_message(LOG_FILE, "a", msg_buf);

   app = _add_client(app, t, reply, "APP");

   /* Notify all GUI clients to add APP */
   EINA_LIST_FOREACH(gui, l, st)
     {
        sprintf(msg_buf, "\t<%p> Sending APP_ADD to <%p>",
              reply, (void *) (uint32_t) st->ptr);
        log_message(LOG_FILE, "a", msg_buf);
        ecore_con_eet_send((void *) (uint32_t) st->ptr,
              CLOUSEAU_APP_ADD_STR, &m);
     }
}

void
_data_req_cb(EINA_UNUSED void *data, Ecore_Con_Reply *reply,
      EINA_UNUSED const char *protocol_name, void *value)
{  /* msg coming from GUI, FWD this to APP specified in REQ */
   data_req_st *req = value;
   sprintf(msg_buf, "\n<%s> msg from <%p>", __func__, reply);
   log_message(LOG_FILE, "a", msg_buf);

   if (req->app)
     {  /* Requesting specific app data */
        if(eina_list_search_unsorted(app,
                 _client_ptr_cmp,
                 (void *) (uintptr_t) req->app))
          {  /* Do the req only of APP connected to daemon */
             data_req_st t = {
                  (unsigned long long) (uintptr_t) reply,
                  (unsigned long long) (uintptr_t) req->app };

             sprintf(msg_buf, "\t<%p> Sending DATA_REQ to <%p>",
                   reply, (void *) (uint32_t) req->app);
             log_message(LOG_FILE, "a", msg_buf);
             ecore_con_eet_send((void *) (uint32_t) req->app,
                   CLOUSEAU_DATA_REQ_STR, &t);
          }
     }
   else
     {  /* requesting ALL apps data */
        Eina_List *l;
        app_info_st *st;
        data_req_st t = {
             (unsigned long long) (uintptr_t) reply,
             (unsigned long long) (uintptr_t) NULL };

        EINA_LIST_FOREACH(app, l, st)
          {
             t.app = (unsigned long long) (uintptr_t) st->ptr;
             sprintf(msg_buf, "\t<%p> Sending DATA_REQ to <%p>",
                   reply, (void *) (uint32_t) st->ptr);
             log_message(LOG_FILE, "a", msg_buf);
             ecore_con_eet_send((void *) (uint32_t) st->ptr, CLOUSEAU_DATA_REQ_STR, &t);
          }
     }
}

void
_tree_data_cb(EINA_UNUSED void *data, EINA_UNUSED Ecore_Con_Reply *reply,
      EINA_UNUSED const char *protocol_name, void *value)
{  /* Tree Data comes from APP, GUI client specified in msg */
   tree_data_st *td = value;
   sprintf(msg_buf, "\n<%s> msg from <%p>", __func__, reply);
   log_message(LOG_FILE, "a", msg_buf);

   if (td->gui)
     {  /* Sending tree data to specific GUI client */
        if(eina_list_search_unsorted(gui,
                 _client_ptr_cmp,
                 (void *) (uintptr_t) td->gui))
          {  /* Do the req only of GUI connected to daemon */
             sprintf(msg_buf, "\t<%p> Sending TREE_DATA to <%p>",
                   reply, (void *) (uint32_t) td->gui);
             log_message(LOG_FILE, "a", msg_buf);
             ecore_con_eet_send((void *) (uint32_t) td->gui,
                   CLOUSEAU_TREE_DATA_STR, value);
          }
     }
   else
     {  /* Sending tree data to all GUI clients */
        Eina_List *l;
        app_info_st *info;
        EINA_LIST_FOREACH(gui, l, info)
          {
             sprintf(msg_buf, "\t<%p> Sending TREE_DATA to <%p>",
                   reply, (void *) (uint32_t) info->ptr);
             log_message(LOG_FILE, "a", msg_buf);
             ecore_con_eet_send((void *) (uint32_t) info->ptr,
                   CLOUSEAU_TREE_DATA_STR, value);
          }
     }

   clouseau_data_tree_free(td->tree);
}

void
_highlight_cb(EINA_UNUSED void *data, EINA_UNUSED Ecore_Con_Reply *reply,
      EINA_UNUSED const char *protocol_name, void *value)
{  /* FWD this message to APP */
   highlight_st *ht = value;
   sprintf(msg_buf, "\n<%s> msg from <%p>", __func__, reply);
   log_message(LOG_FILE, "a", msg_buf);

   if(eina_list_search_unsorted(app,
            _client_ptr_cmp, (void *) (uintptr_t) ht->app))
     {  /* Do the REQ only of APP connected to daemon */
        sprintf(msg_buf, "\t<%p> Sending HIGHLIGHT to <%p>",
              reply, (void *) (uint32_t) ht->app);
        log_message(LOG_FILE, "a", msg_buf);
        ecore_con_eet_send((void *) (uint32_t) ht->app,
              CLOUSEAU_HIGHLIGHT_STR, value);
     }
}

void
_bmp_req_cb(EINA_UNUSED void *data, Ecore_Con_Reply *reply,
      EINA_UNUSED const char *protocol_name, void *value)
{  /* BMP data request coming from GUI to APP client */
   bmp_req_st *req = value;
   sprintf(msg_buf, "\n<%s> msg from <%p>", __func__, reply);
   log_message(LOG_FILE, "a", msg_buf);

   if(eina_list_search_unsorted(app,
            _client_ptr_cmp, (void *) (uintptr_t) req->app))
     {  /* Do the req only if APP connected to daemon */
        bmp_req_st t = {
             (unsigned long long) (uintptr_t) reply,
             req->app, req->object, req->ctr };

        sprintf(msg_buf, "\t<%p> Sending BMP_REQ to <%p>",
              reply, (void *) (uint32_t) req->app);
        log_message(LOG_FILE, "a", msg_buf);
        ecore_con_eet_send((void *) (uint32_t) req->app,
              CLOUSEAU_BMP_REQ_STR, &t);
     }
}

void
_bmp_data_cb(EINA_UNUSED void *data, EINA_UNUSED Ecore_Con_Reply *reply,
      EINA_UNUSED const char *protocol_name, EINA_UNUSED const char *section,
      void *value, EINA_UNUSED size_t length)
{  /* BMP Data comes from APP, GUI client specified in msg */
   bmp_info_st *st = clouseau_data_packet_info_get(protocol_name,
         value, length);

   sprintf(msg_buf, "\n<%s> msg from <%p>", __func__, reply);
   log_message(LOG_FILE, "a", msg_buf);

   if (st->gui)
     {  /* Sending BMP data to specific GUI client */
        if(eina_list_search_unsorted(gui,
                 _client_ptr_cmp,
                 (void *) (uintptr_t) st->gui))
          {  /* Do the req only of GUI connected to daemon */
             sprintf(msg_buf, "\t<%p> Sending BMP_DATA to <%p>",
                   reply, (void *) (uint32_t) st->gui);
             log_message(LOG_FILE, "a", msg_buf);
             ecore_con_eet_raw_send((void *) (uint32_t) st->gui,
                   CLOUSEAU_BMP_DATA_STR, "BMP", value, length);
          }
     }
   else
     {  /* Sending BMP data to all GUI clients */
        Eina_List *l;
        app_info_st *info;
        EINA_LIST_FOREACH(gui, l, info)
          {
             sprintf(msg_buf, "\t<%p> Sending BMP_DATA to <%p>",
                   reply, (void *) (uint32_t) info->ptr);
             log_message(LOG_FILE, "a", msg_buf);
             ecore_con_eet_raw_send((void *) (uint32_t) info->ptr,
                   CLOUSEAU_BMP_DATA_STR, "BMP", value, length);
          }
     }

   if (st->bmp)
     free(st->bmp);

   free(st);
}
/* END   - Ecore communication callbacks */

int main(void)
{
   daemonize();
   eina_init();
   ecore_init();
   ecore_con_init();
   clouseau_data_init();
   Ecore_Con_Server *server = NULL;

   if (!(server = ecore_con_server_add(ECORE_CON_REMOTE_TCP,
               LISTEN_IP, PORT, NULL)))
     exit(1);

   eet_svr = ecore_con_eet_server_new(server);
   if (!eet_svr)
     exit(2);

   clouseau_register_descs(eet_svr);

   /* Register callbacks for ecore_con_eet */
   ecore_con_eet_client_connect_callback_add(eet_svr, _add, NULL);
   ecore_con_eet_client_disconnect_callback_add(eet_svr, _del, server);
   ecore_con_eet_data_callback_add(eet_svr, CLOUSEAU_GUI_CLIENT_CONNECT_STR,
         _gui_client_connect_cb, NULL);
   ecore_con_eet_data_callback_add(eet_svr, CLOUSEAU_APP_CLIENT_CONNECT_STR,
         _app_client_connect_cb, NULL);
   ecore_con_eet_data_callback_add(eet_svr, CLOUSEAU_DATA_REQ_STR,
         _data_req_cb, NULL);
   ecore_con_eet_data_callback_add(eet_svr, CLOUSEAU_TREE_DATA_STR,
         _tree_data_cb, NULL);
   ecore_con_eet_data_callback_add(eet_svr, CLOUSEAU_HIGHLIGHT_STR,
         _highlight_cb, NULL);
   ecore_con_eet_data_callback_add(eet_svr, CLOUSEAU_BMP_REQ_STR,
         _bmp_req_cb, NULL);
   ecore_con_eet_raw_data_callback_add(eet_svr, CLOUSEAU_BMP_DATA_STR,
         _bmp_data_cb, NULL);

   ecore_main_loop_begin();
   _daemon_cleanup();

   return 0;
}
