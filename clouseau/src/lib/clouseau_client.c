#include "Clouseau.h"
#include <dlfcn.h>
#include <execinfo.h>

#include <Ecore_Con_Eet.h>
#include <Edje.h>
#include <Evas.h>
#include <Elementary.h>
#include <Ecore_X.h>

#include "clouseau_private.h"

static Eina_Bool _elm_is_init = EINA_FALSE;
static const char *_my_app_name = NULL;

static void
libclouseau_item_add(Evas_Object *o, Clouseau_Tree_Item *parent)
{
   Clouseau_Tree_Item *treeit;
   Eina_List *children;
   Evas_Object *child;

   treeit = calloc(1, sizeof(Clouseau_Tree_Item));
   if (!treeit) return ;

   treeit->ptr = (uintptr_t) o;
   treeit->is_obj = EINA_TRUE;

   treeit->name = eina_stringshare_add(evas_object_type_get(o));
   treeit->is_clipper = !!evas_object_clipees_get(o);
   treeit->is_visible = evas_object_visible_get(o);
   treeit->info = clouseau_object_information_get(treeit);

   parent->children = eina_list_append(parent->children, treeit);

   /* if (!evas_object_smart_data_get(o)) return ; */

   /* Do this only for smart object */
   children = evas_object_smart_members_get(o);
   EINA_LIST_FREE(children, child)
     libclouseau_item_add(child, treeit);
}

static void *
_canvas_bmp_get(Ecore_Evas *ee, Evas_Coord *w_out, Evas_Coord *h_out)
{
   Ecore_X_Image *img;
   Ecore_X_Window_Attributes att;
   unsigned char *src;
   unsigned int *dst;
   int bpl = 0, rows = 0, bpp = 0;
   Evas_Coord w, h;

   /* Check that this window still exists */
   Eina_List *eeitr, *ees = ecore_evas_ecore_evas_list_get();
   Ecore_Evas *eel;
   Eina_Bool found_evas = EINA_FALSE;

   EINA_LIST_FOREACH(ees, eeitr, eel)
      if (eel == ee)
        {
           found_evas = EINA_TRUE;
           break;
        }

   Ecore_X_Window xwin = (found_evas) ?
      (Ecore_X_Window) ecore_evas_window_get(ee) : 0;

   if (!xwin)
     {
        printf("Can't grab X window.\n");
        *w_out = *h_out = 0;
        return NULL;
     }

   Evas *e = ecore_evas_get(ee);
   evas_output_size_get(e, &w, &h);
   memset(&att, 0, sizeof(Ecore_X_Window_Attributes));
   ecore_x_window_attributes_get(xwin, &att);
   img = ecore_x_image_new(w, h, att.visual, att.depth);
   ecore_x_image_get(img, xwin, 0, 0, 0, 0, w, h);
   src = ecore_x_image_data_get(img, &bpl, &rows, &bpp);
   dst = malloc(w * h * sizeof(int));  /* Will be freed by the user */
   if (!ecore_x_image_is_argb32_get(img))
     {  /* Fill dst buffer with image convert */
        ecore_x_image_to_argb_convert(src, bpp, bpl,
              att.colormap, att.visual,
              0, 0, w, h,
              dst, (w * sizeof(int)), 0, 0);
     }
   else
     {  /* Fill dst buffer by copy */
        memcpy(dst, src, (w * h * sizeof(int)));
     }

   /* dst now holds window bitmap */
   ecore_x_image_free(img);
   *w_out = w;
   *h_out = h;
   return (void *) dst;
}

static Eina_List *
_load_list(void)
{
   Eina_List *tree = NULL;
   Eina_List *ees;
   Ecore_Evas *ee;

   ees = ecore_evas_ecore_evas_list_get();

   EINA_LIST_FREE(ees, ee)
     {
        Eina_List *objs;
        Evas_Object *obj;
        Clouseau_Tree_Item *treeit;

        Evas *e;
        int w, h;

        e = ecore_evas_get(ee);
        evas_output_size_get(e, &w, &h);

        treeit = calloc(1, sizeof(Clouseau_Tree_Item));
        if (!treeit) continue ;

        treeit->name = eina_stringshare_add(ecore_evas_title_get(ee));
        treeit->ptr = (uintptr_t) ee;

        tree = eina_list_append(tree, treeit);

        objs = evas_objects_in_rectangle_get(e, SHRT_MIN, SHRT_MIN,
              USHRT_MAX, USHRT_MAX, EINA_TRUE, EINA_TRUE);

        EINA_LIST_FREE(objs, obj)
          libclouseau_item_add(obj, treeit);
    }

   return tree;  /* User has to call clouseau_tree_free() */
}

Eina_Bool
_add(EINA_UNUSED void *data, Ecore_Con_Reply *reply,
      EINA_UNUSED Ecore_Con_Server *conn)
{
/*   ecore_con_server_data_size_max_set(conn, -1); */
   connect_st t = { getpid(), _my_app_name };
   ecore_con_eet_send(reply, CLOUSEAU_APP_CLIENT_CONNECT_STR, &t);

   return ECORE_CALLBACK_RENEW;
}

Eina_Bool
_del(EINA_UNUSED void *data, EINA_UNUSED Ecore_Con_Reply *reply,
      Ecore_Con_Server *conn)
{
   if (!conn)
     {
        printf("Failed to establish connection to the server.\nExiting.\n");
        ecore_main_loop_quit();
        return ECORE_CALLBACK_RENEW;
     }

   printf("Lost server with ip <%s>\n", ecore_con_server_ip_get(conn));

   ecore_con_server_del(conn);

   ecore_main_loop_quit();
   return ECORE_CALLBACK_RENEW;
}

void
_data_req_cb(EINA_UNUSED void *data, Ecore_Con_Reply *reply,
      EINA_UNUSED const char *protocol_name, void *value)
{  /* data req includes ptr to GUI, to tell which client asking */
   data_req_st *req = value;
   tree_data_st t;
   t.gui = req->gui;  /* GUI client requesting data from daemon */
   t.app = req->app;  /* APP client sending data to daemon */
   t.tree = _load_list();

   if (t.tree)
     {  /* Reply with tree data to data request */
        ecore_con_eet_send(reply, CLOUSEAU_TREE_DATA_STR, &t);
        clouseau_data_tree_free(t.tree);
     }
}

void
_highlight_cb(EINA_UNUSED void *data, EINA_UNUSED Ecore_Con_Reply *reply,
      EINA_UNUSED const char *protocol_name, void *value)
{  /* Highlight msg contains PTR of object to highlight */
   highlight_st *ht = value;
   Evas_Object *obj = (Evas_Object *) (uintptr_t) ht->object;
   clouseau_data_object_highlight(obj, NULL, NULL);
}

void
_bmp_req_cb(EINA_UNUSED void *data, EINA_UNUSED Ecore_Con_Reply *reply,
      EINA_UNUSED const char *protocol_name, void *value)
{  /* Bitmap req msg contains PTR of Ecore Evas */
   bmp_req_st *req = value;
   Evas_Coord w, h;
   unsigned int size = 0;
   void *bmp = _canvas_bmp_get((Ecore_Evas *) (uintptr_t)
         req->object, &w, &h);

   bmp_info_st t = { req->gui,
        req->app, req->object , req->ctr, w, h,
        NULL,NULL, NULL, 1.0,
        NULL, NULL, NULL, NULL, NULL, NULL };

   void *p = clouseau_data_packet_compose(CLOUSEAU_BMP_DATA_STR,
         &t, &size, bmp, (w * h * sizeof(int)));


   if (p)
     {
        ecore_con_eet_raw_send(reply, CLOUSEAU_BMP_DATA_STR, "BMP", p, size);
        free(p);
     }

   if (bmp)
     free(bmp);
}

static Eina_Bool
_connect_to_daemon(void)
{
   Ecore_Con_Server *server;
   const char *address = LOCALHOST;
   Ecore_Con_Eet *eet_svr = NULL;

   eina_init();
   ecore_init();
   ecore_con_init();

   server = ecore_con_server_connect(ECORE_CON_REMOTE_TCP,
         LOCALHOST, PORT, NULL);

   if (!server)
     {
        printf("could not connect to the server: %s, port %d.\n",
              address, PORT);
        return EINA_FALSE;
     }

   eet_svr = ecore_con_eet_client_new(server);
   if (!eet_svr)
     {
        printf("could not create con_eet client.\n");
        return EINA_FALSE;
     }

   clouseau_register_descs(eet_svr);

   /* Register callbacks for ecore_con_eet */
   ecore_con_eet_server_connect_callback_add(eet_svr, _add, NULL);
   ecore_con_eet_server_disconnect_callback_add(eet_svr, _del, NULL);
   ecore_con_eet_data_callback_add(eet_svr, CLOUSEAU_DATA_REQ_STR,
         _data_req_cb, NULL);
   ecore_con_eet_data_callback_add(eet_svr, CLOUSEAU_HIGHLIGHT_STR,
         _highlight_cb, NULL);
   ecore_con_eet_data_callback_add(eet_svr, CLOUSEAU_BMP_REQ_STR,
         _bmp_req_cb, NULL);

   return EINA_TRUE;
}

/* Hook on the elm_init
 * We only do something here if we didn't already go into elm_init,
 * which probably means we are not using elm. */
EAPI int
elm_init(int argc, char **argv)
{
   int (*_elm_init)(int, char **) = dlsym(RTLD_NEXT, __func__);

   if (!_elm_is_init)
     {
        _my_app_name = argv[0];
        _elm_is_init = EINA_TRUE;
     }

   return _elm_init(argc, argv);
}

/* Hook on the main loop
 * We only do something here if we didn't already go into elm_init,
 * which probably means we are not using elm. */
EAPI void
ecore_main_loop_begin(void)
{
   void (*_ecore_main_loop_begin)(void) = dlsym(RTLD_NEXT, __func__);

   if (!_elm_is_init)
     {
        _my_app_name = "clouseau";
     }

   clouseau_data_init();

   if(!_connect_to_daemon())
     {
        printf("Failed to connect to server.\n");
        return;
     }

   _ecore_main_loop_begin();

   clouseau_data_shutdown();

   return;
}

#define EINA_LOCK_DEBUG_BT_NUM 64
typedef void (*Eina_Lock_Bt_Func) ();

EAPI Evas_Object *
evas_object_new(Evas *e)
{
   Eina_Lock_Bt_Func lock_bt[EINA_LOCK_DEBUG_BT_NUM];
   int lock_bt_num;
   Evas_Object *(*_evas_object_new)(Evas *e) = dlsym(RTLD_NEXT, __func__);
   Eina_Strbuf *str;
   Evas_Object *r;
   char **strings;
   int i;

   r = _evas_object_new(e);
   if (!r) return NULL;

   lock_bt_num = backtrace((void **)lock_bt, EINA_LOCK_DEBUG_BT_NUM);
   strings = backtrace_symbols((void **)lock_bt, lock_bt_num);

   str = eina_strbuf_new();

   for (i = 1; i < lock_bt_num; ++i)
     eina_strbuf_append_printf(str, "%s\n", strings[i]);

   evas_object_data_set(r, ".clouseau.bt", eina_stringshare_add(eina_strbuf_string_get(str)));

   free(strings);
   eina_strbuf_free(str);

   return r;
}

EAPI void
evas_object_free(Evas_Object *obj, int clean_layer)
{
   void (*_evas_object_free)(Evas_Object *obj, int clean_layer) = dlsym(RTLD_NEXT, __func__);
   const char *tmp;

   tmp = evas_object_data_get(obj, ".clouseau.bt");
   eina_stringshare_del(tmp);

   _evas_object_free(obj, clean_layer);
}
