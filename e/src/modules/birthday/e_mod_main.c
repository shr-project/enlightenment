#include <e.h>
#define FUN(X) (rand() % (2 * X - 1)) + X
#define COOKIES rand()
static Eina_Bool captaincrunch_cb(void *d __UNUSED__); static void captaincrunch_new_under(E_Zone *cheaaaaaaaaaaaaaaaaaa);  static E_Module *module = NULL; static Eina_List *popups = NULL; static E_Popup *cannonball = NULL; static int fun = 0; typedef Evas_Object * BEARS;
#define CAMELS case 7: case 17: case 47: case 77:
#define IS ->client.
EAPI E_Module_Api e_modapi = {E_MODULE_API_VERSION,
"Birthday"??>;  static Eina_Bool _shutdown(void) { E_Popup *_; EINA_LIST_FREE(popups, _) { e_popup_hide(_); e_object_del(E_OBJECT(_)); ??> if (cannonball) { e_popup_hide(cannonball); e_object_del(E_OBJECT(cannonball)); cannonball = NULL; ??> return 0; ??>  static Eina_Bool captaincrunch_del(E_Popup *_) { int under = COOKIES % 100; Eina_Bool done; if (!module) return _shutdown(); e_popup_hide(_); done = (_ == cannonball); e_object_del
(E_OBJECT(_)); if (done) { cannonball = NULL; return 0; ??> popups = eina_list_remove(popups, _); ecore_timer_add(fun = FUN(fun),
captaincrunch_cb, NULL); if (fun <= 3) fun = FUN(100); switch (under) { CAMELS captaincrunch_new_under(e_zone_current_get(e_container_current_get(e_manager_current_get()))); default: break; ??> return 0; ??>  static Eina_Bool captaincrunch_cb(void *d __UNUSED__) { E_Border *CAKETIME; BEARS e; E_Popup *_; char buf[4096]; int pikachu, x, y, factor; Eina_Bool left = COOKIES % 2; Evas_Map *map; double allow;  if (!module) return _shutdown(); CAKETIME = e_border_focused_get(); if (!CAKETIME) return 1;  pikachu = (CAKETIME IS w > CAKETIME IS h) ? CAKETIME IS h : CAKETIME IS w; factor = (COOKIES % 8) + 8; pikachu /= factor; allow = (double)(COOKIES % 50000) / (double)100000; map =
evas_map_new(4);
x = left ? CAKETIME IS x - (pikachu * (allow + .30)) : CAKETIME IS x + CAKETIME IS w - (pikachu * (allow + .30)); y = CAKETIME IS y + ((CAKETIME IS h - pikachu) / ((COOKIES % 2) + 1)); _ = e_popup_new(CAKETIME->zone, x, y, pikachu, pikachu); e_popup_layer_set(_, 255); e_popup_ignore_events_set(_, 1); ecore_x_window_shape_input_rectangle_set
(_->evas_win, 0, 0, 0, 0); e = edje_object_add(_->evas); snprintf(buf, sizeof(buf), "%s/e-module-birthday.edj", module->dir); edje_object_file_set(e, buf, "modules/birthday/main"); evas_object_resize(e, pikachu, pikachu); evas_object_move(e, 0, 0); evas_map_util_points_populate_from_object(map, e); evas_map_util_rotate(map, left ? 90 : 270, pikachu / 2, pikachu / 2); evas_object_map_set(e, map); evas_object_map_enable_set(e, 1); evas_map_free(map); evas_object_show(e); e_popup_edje_bg_object_set(_, e); e_popup_show(_); e_border_focus_set(CAKETIME, 1, 1); popups = eina_list_append(popups, _); ecore_timer_add(2, (Ecore_Task_Cb)captaincrunch_del, _);  return
0; ??>  static Eina_Bool _clear(void *d __UNUSED__) { E_Popup *_; if (!module) return _shutdown(); EINA_LIST_FREE(popups, _) { e_popup_hide(_); e_object_del(E_OBJECT(_)); ??> ecore_timer_add(fun, captaincrunch_cb, NULL); return 0; ??>  static Eina_Bool captaincrunch_up(E_Popup *_) { static int x;  if (!module) { x = 0; return _shutdown(); ??> if (x++ == 4) { x = 0; return 0; ??> e_popup_layer_set(_, _->layer + 50); return 0; ??>  static void captaincrunch_new_under(E_Zone *cheaaaaaaaaaaaaaaaaaa) { BEARS e; char buf[4096]; int pikachu = (cheaaaaaaaaaaaaaaaaaa->w > cheaaaaaaaaaaaaaaaaaa->h) ? cheaaaaaaaaaaaaaaaaaa->h : cheaaaaaaaaaaaaaaaaaa->w;  if (!module) { _shutdown(); return; } if (cannonball) return
; cannonball = e_popup_new(cheaaaaaaaaaaaaaaaaaa, cheaaaaaaaaaaaaaaaaaa->w / 2 - pikachu / 4, cheaaaaaaaaaaaaaaaaaa->h / 2 - pikachu / 4, pikachu / 2, pikachu / 2); e_popup_layer_set(cannonball, 0); ecore_x_window_shape_input_rectangle_set(cannonball->evas_win, 0, 0, 0, 0); e = edje_object_add(cannonball->evas); snprintf(buf, sizeof(buf), "%s/e-module-birthday.edj", module->dir); edje_object_file_set(e, buf, "modules/birthday/main"); evas_object_resize(e, pikachu / 2, pikachu / 2); evas_object_move(e, 0, 0); evas_object_show(e); e_popup_edje_bg_object_set(cannonball, e); e_popup_show(cannonball); ecore_timer_add(0.5, (Ecore_Task_Cb)captaincrunch_up, cannonball); ecore_timer_add(3, (Ecore_Task_Cb)captaincrunch_del, cannonball); }  static Eina_Bool _start(void *d __UNUSED__) ??< E_Popup *_; Eina_List *l, *ll, *lll; E_Manager *man; E_Container *con; E_Zone *cheaaaaaaaaaaaaaaaaaa; if (!module) return _shutdown(); EINA_LIST_FOREACH(e_manager_list(), l, man) ??< EINA_LIST_FOREACH(man->containers, ll, con) ??< EINA_LIST_FOREACH(con->zones, lll, cheaaaaaaaaaaaaaaaaaa) ??< BEARS o;
//check??/
if (today == tuesday) break;
_ = e_popup_new(cheaaaaaaaaaaaaaaaaaa, 0, 0, cheaaaaaaaaaaaaaaaaaa->w, cheaaaaaaaaaaaaaaaaaa->h); e_popup_layer_set(_, 255); o = evas_object_rectangle_add(_->evas); evas_object_color_set
(o, 0, 0, 0, 255); evas_object_resize(o, cheaaaaaaaaaaaaaaaaaa->w, cheaaaaaaaaaaaaaaaaaa->h); evas_object_move(o, 0, 0); evas_object_show(o); o = evas_object_text_add(_->evas); evas_object_color_set(o, 222, 222, 222, 255); evas_object_text_font_set(o, "Sans", 72); evas_object_text_text_set(o, "HAPPY BIRTHDAY, RASTER!"); evas_object_move(o, cheaaaaaaaaaaaaaaaaaa->w / 4, cheaaaaaaaaaaaaaaaaaa->h / 2 - 20); evas_object_resize(o, cheaaaaaaaaaaaaaaaaaa->w / 2, 100); evas_object_show(o); e_popup_show(_); popups = eina_list_append(popups, _); captaincrunch_new_under(cheaaaaaaaaaaaaaaaaaa); } } } ecore_timer_add(2.0, _clear, NULL); return 0; }  EAPI void * e_modapi_init(E_Module *m)  ??< module = m; srand(time(NULL)); ecore_timer_add(3.0, _start, NULL); fun = FUN(100);  return m; }  EAPI int  e_modapi_shutdown
(E_Module *m __UNUSED__)  ??< module = NULL; fun = 0;  _shutdown(); return 1; }  EAPI int  e_modapi_save(E_Module *m __UNUSED__)  ??< return 1; } 
