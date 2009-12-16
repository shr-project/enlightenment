#include "e.h"
#include "e_mod_main.h"
#include "e_mod_win.h"
#include "e_mod_config.h"

/* local function prototypes */
static void _il_ind_win_cb_free(Il_Ind_Win *iwin);
static void _il_ind_win_cb_resize(E_Win *ewin);
static void _il_ind_win_cb_mouse_down(void *data, Evas *evas, Evas_Object *obj, void *event);
static int _il_ind_win_gadcon_client_add(void *data, const E_Gadcon_Client_Class *cc);
static void _il_ind_win_gadcon_client_del(void *data, E_Gadcon_Client *gcc);
static void _il_ind_win_gadcon_min_size_request(void *data, E_Gadcon *gc, Evas_Coord w, Evas_Coord h);
static void _il_ind_win_gadcon_size_request(void *data, E_Gadcon *gc, Evas_Coord w, Evas_Coord h);
static Evas_Object *_il_ind_win_gadcon_frame_request(void *data, E_Gadcon_Client *gcc, const char *style);
static void _il_ind_win_cb_menu_post(void *data, E_Menu *m);
static void _il_ind_win_menu_append(Il_Ind_Win *iwin, E_Menu *mn);
static void _il_ind_win_cb_menu_pre(void *data, E_Menu *mn);
static void _il_ind_win_cb_menu_items_append(void *data, E_Gadcon_Client *gcc, E_Menu *mn);
static void _il_ind_win_cb_menu_edit(void *data, E_Menu *mn, E_Menu_Item *mi);
static void _il_ind_win_cb_menu_contents(void *data, E_Menu *mn, E_Menu_Item *mi);

int 
e_mod_ind_win_init(void) 
{
   return 1;
}

int 
e_mod_ind_win_shutdown(void) 
{
   return 1;
}

Il_Ind_Win *
e_mod_ind_win_new(void) 
{
   Il_Ind_Win *iwin;
   E_Zone *zone;
   Evas *evas;
   Eina_List *l;
   E_Config_Gadcon *cg;
   Ecore_X_Window_State states[2];
   char buff[PATH_MAX];

   iwin = E_OBJECT_ALLOC(Il_Ind_Win, IL_IND_WIN_TYPE, _il_ind_win_cb_free);
   if (!iwin) return NULL;

   snprintf(buff, sizeof(buff), "%s/e-module-illume-indicator.edj", 
            il_ind_cfg->mod_dir);

   iwin->win = e_win_new(e_util_container_number_get(0));
   states[0] = ECORE_X_WINDOW_STATE_SKIP_TASKBAR;
   states[1] = ECORE_X_WINDOW_STATE_SKIP_PAGER;
   ecore_x_netwm_window_state_set(iwin->win->evas_win, states, 2);
   ecore_x_icccm_hints_set(iwin->win->evas_win, 0, 0, 0, 0, 0, 0, 0);
   ecore_x_netwm_window_type_set(iwin->win->evas_win, ECORE_X_WINDOW_TYPE_DOCK);

   e_win_no_remember_set(iwin->win, 1);
   e_win_resize_callback_set(iwin->win, _il_ind_win_cb_resize);
   e_win_borderless_set(iwin->win, 1);
   iwin->win->data = iwin;
   e_win_title_set(iwin->win, _("Illume Indicator"));
   e_win_name_class_set(iwin->win, "Illume-Indicator", "Illume-Indicator");

   evas = e_win_evas_get(iwin->win);

   zone = e_util_container_zone_number_get(0, 0);

   iwin->o_event = evas_object_rectangle_add(evas);
   evas_object_color_set(iwin->o_event, 0, 0, 0, 0);
   evas_object_move(iwin->o_event, 0, 0);
   evas_object_resize(iwin->o_event, zone->w, 32);
   evas_object_event_callback_add(iwin->o_event, EVAS_CALLBACK_MOUSE_DOWN, 
                                  _il_ind_win_cb_mouse_down, iwin);
   evas_object_show(iwin->o_event);
   evas_object_layer_set(iwin->o_event, 200);

   iwin->o_base = edje_object_add(evas);
   evas_object_resize(iwin->o_base, zone->w, 32);
   if (!e_theme_edje_object_set(iwin->o_base, 
                                "base/theme/modules/illume-indicator", 
                                "modules/illume-indicator/shelf"))
     edje_object_file_set(iwin->o_base, buff, "modules/illume-indicator/shelf");
   evas_object_move(iwin->o_base, 0, 0);
   evas_object_resize(iwin->o_base, zone->w, 32);
   evas_object_layer_set(iwin->o_base, 200);
   evas_object_show(iwin->o_base);

   iwin->gadcon = e_gadcon_swallowed_new("illume-indicator", 1, iwin->o_base, 
                                         "e.swallow.content");
   iwin->gadcon->instant_edit = 1;
   edje_extern_object_min_size_set(iwin->gadcon->o_container, zone->w, 32);
   edje_object_part_swallow(iwin->o_base, "e.swallow.content", 
                            iwin->gadcon->o_container);
   e_gadcon_min_size_request_callback_set(iwin->gadcon, 
                                          _il_ind_win_gadcon_min_size_request, 
                                          iwin);
   e_gadcon_size_request_callback_set(iwin->gadcon, 
                                      _il_ind_win_gadcon_size_request, iwin);
   e_gadcon_frame_request_callback_set(iwin->gadcon, 
                                       _il_ind_win_gadcon_frame_request, iwin);
   e_gadcon_orient(iwin->gadcon, E_GADCON_ORIENT_FLOAT);
   e_gadcon_zone_set(iwin->gadcon, zone);
   e_gadcon_ecore_evas_set(iwin->gadcon, iwin->win->ecore_evas);
   e_gadcon_util_menu_attach_func_set(iwin->gadcon, 
                                      _il_ind_win_cb_menu_items_append, iwin);

   e_gadcon_populate(iwin->gadcon);

   e_win_size_min_set(iwin->win, zone->w, 32);
   e_win_show(iwin->win);
   e_win_move_resize(iwin->win, 0, 0, zone->w, 32);

   return iwin;
}

/* local function prototypes */
static void 
_il_ind_win_cb_free(Il_Ind_Win *iwin) 
{
   if (iwin->menu) 
     {
        e_menu_post_deactivate_callback_set(iwin->menu, NULL, NULL);
        e_object_del(E_OBJECT(iwin->menu));
        iwin->menu = NULL;
     }

   if (iwin->o_event) evas_object_del(iwin->o_event);
   if (iwin->o_base) evas_object_del(iwin->o_base);

   e_object_del(E_OBJECT(iwin->gadcon));
   iwin->gadcon = NULL;

   e_object_del(E_OBJECT(iwin->win));
   E_FREE(iwin);
}

static void 
_il_ind_win_cb_resize(E_Win *ewin) 
{
   Il_Ind_Win *iwin;

   if (!(iwin = ewin->data)) return;
   evas_object_resize(iwin->o_event, iwin->win->w, iwin->win->h);
   evas_object_resize(iwin->o_base, iwin->win->w, iwin->win->h);
}

static void 
_il_ind_win_cb_mouse_down(void *data, Evas *evas, Evas_Object *obj, void *event) 
{
   Il_Ind_Win *iwin;
   Evas_Event_Mouse_Down *ev;

   if (!(iwin = data)) return;
   ev = event;
   if (ev->button == 1) 
     {
        iwin->drag.x = ev->output.x;
        iwin->drag.y = ev->output.y;
        iwin->drag.start = 1;
        iwin->drag.dnd = 0;
        ecore_x_e_illume_drag_set(iwin->win->border->client.win, 1);
        ecore_x_e_illume_drag_start_send(iwin->win->border->client.win);
     }
   else if (ev->button == 3) 
     {
        E_Menu *mn;
        E_Zone *zone;
        int x, y, w, h;

        mn = e_menu_new();
        e_menu_post_deactivate_callback_set(mn, _il_ind_win_cb_menu_post, iwin);
        iwin->menu = mn;

        _il_ind_win_menu_append(iwin, mn);

        zone = e_util_container_zone_number_get(0, 0);
        e_gadcon_canvas_zone_geometry_get(iwin->gadcon, &x, &y, NULL, NULL);
        e_menu_activate_mouse(mn, zone, x + ev->output.x, y + ev->output.y, 
                              1, 1, E_MENU_POP_DIRECTION_AUTO, ev->timestamp);
     }
}

static void 
_il_ind_win_cb_menu_post(void *data, E_Menu *m) 
{
   Il_Ind_Win *iwin;

   if (!(iwin = data)) return;
   if (!iwin->menu) return;
   e_object_del(E_OBJECT(iwin->menu));
   iwin->menu = NULL;
}

static void 
_il_ind_win_cb_menu_items_append(void *data, E_Gadcon_Client *gcc, E_Menu *mn) 
{
   Il_Ind_Win *iwin;

   if (!(iwin = data)) return;
   _il_ind_win_menu_append(iwin, mn);
}

static void 
_il_ind_win_menu_append(Il_Ind_Win *iwin, E_Menu *mn) 
{
   E_Menu *sm;
   E_Menu_Item *mi;

   sm = e_menu_new();
   mi = e_menu_item_new(mn);
   e_menu_item_label_set(mi, _("Illume Indicator"));
   e_util_menu_item_theme_icon_set(mi, "preferences-desktop-shelf");
   e_menu_pre_activate_callback_set(sm, _il_ind_win_cb_menu_pre, iwin);
   e_object_data_set(E_OBJECT(mi), iwin);
   e_menu_item_submenu_set(mi, sm);
}

static void 
_il_ind_win_cb_menu_pre(void *data, E_Menu *mn) 
{
   Il_Ind_Win *iwin;
   E_Menu_Item *mi;

   if (!(iwin = data)) return;
   e_menu_pre_activate_callback_set(mn, NULL, NULL);

   mi = e_menu_item_new(mn);
   if (iwin->gadcon->editing)
     e_menu_item_label_set(mi, _("Stop Moving/Resizing Items"));
   else
     e_menu_item_label_set(mi, _("Begin Moving/Resizing Items"));
   e_util_menu_item_theme_icon_set(mi, "transform-scale");
   e_menu_item_callback_set(mi, _il_ind_win_cb_menu_edit, iwin);

   mi = e_menu_item_new(mn);
   e_menu_item_separator_set(mi, 1);

   mi = e_menu_item_new(mn);
   e_menu_item_label_set(mi, _("Set Contents"));
   e_util_menu_item_theme_icon_set(mi, "preferences-desktop-shelf");
   e_menu_item_callback_set(mi, _il_ind_win_cb_menu_contents, iwin);
}

static void 
_il_ind_win_gadcon_min_size_request(void *data, E_Gadcon *gc, Evas_Coord w, Evas_Coord h) 
{
   Il_Ind_Win *iwin;

   if (!(iwin = data)) return;
   if (gc == iwin->gadcon) 
     {
        if (h < 32) h = 32;
        edje_extern_object_min_size_set(iwin->gadcon->o_container, w, h);
        edje_object_part_swallow(iwin->o_base, "e.swallow.content", 
                                 iwin->gadcon->o_container);
     }
   evas_object_resize(iwin->o_base, iwin->win->w, iwin->win->h);
}

static void 
_il_ind_win_gadcon_size_request(void *data, E_Gadcon *gc, Evas_Coord w, Evas_Coord h) 
{
   return;
}

static Evas_Object *
_il_ind_win_gadcon_frame_request(void *data, E_Gadcon_Client *gcc, const char *style) 
{
   return NULL;
}

static void 
_il_ind_win_cb_menu_edit(void *data, E_Menu *mn, E_Menu_Item *mi) 
{
   Il_Ind_Win *iwin;

   if (!(iwin = data)) return;
   if (iwin->gadcon->editing)
     e_gadcon_edit_end(iwin->gadcon);
   else
     e_gadcon_edit_begin(iwin->gadcon);
}

static void 
_il_ind_win_cb_menu_contents(void *data, E_Menu *mn, E_Menu_Item *mi) 
{
   Il_Ind_Win *iwin;

   if (!(iwin = data)) return;
   if (!iwin->gadcon->config_dialog) 
     e_int_gadcon_config_shelf(iwin->gadcon);
}
