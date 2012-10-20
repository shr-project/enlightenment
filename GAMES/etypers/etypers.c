/*
 * Enlightement Typers.
 *
 * Copyright 2012 Hermet (ChunEon Park)
 *
 * This application is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This application is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include <assert.h>
#include <stdlib.h>
#include <Elementary.h>
#include <Ecore_Input.h>
#include <Ecore.h>
#include <Eina.h>

#define DEFAULT_WIN_W 600
#define DEFAULT_WIN_H 800
#define NEW_ENEMY_DURATION_SEC 5
#define LEVEL_INC_WEIGHT 0.01
#define DROP_DIST_WEIGHT 10
#define DEFENSE_WALL_CNT 15
#define WALL_COLOR 100, 100, 100, 170
#define MINUS_SCORE 25

typedef struct _Enemy Enemy;
typedef struct _AppData AppData;
typedef enum _GameState GameState;

static void _pause(AppData *appdata);
static void _resume(AppData *appdata);
static void _popup(AppData *appdata);
static void _reset_defense_wall(AppData *appdata);

enum _GameState
{
   Ready,
   Playing,
   Paused,
   SubPopup,
   GameOver
};

struct _AppData
{
   Eina_Inlist *enemies;
   Eina_Array *words_array;
   Eina_List *defense_wall;
   double level;
   double last_frame_time;
   double interval_time;
   unsigned long score;
   float hit_cnt;
   float hit_try_cnt;
   unsigned long combo;
   Evas_Coord bound_w;
   Evas_Coord bound_h;
   Evas_Coord_Rectangle defense_rect;
   Evas_Object *win;
   Evas_Object *ly;
   Evas_Object *bx;
   Evas_Object *entry;
   Evas_Object *table;
   Evas_Object *popup;
   GameState state;
   GameState prev_state;
   Ecore_Animator *animator;
};

struct _Enemy
{
   EINA_INLIST;
   Evas_Object *entry;
   float x;
   float y;
   float w;
   float h;
};

static double
level_handicap_for_enemy_add(double level)
{
   return ((double) (rand() % (int) (level * 100000))) * 0.000014;
}

static unsigned long
_enemy_score_get(Enemy* enemy)
{
   return strlen(elm_object_text_get(enemy->entry)) * 10;
}

static void
_remove_all_enemies(AppData *appdata)
{
   while (appdata->enemies)
     {
        Enemy *enemy = EINA_INLIST_CONTAINER_GET(appdata->enemies,
                                                 Enemy);
        evas_object_del(enemy->entry);
        appdata->enemies = eina_inlist_remove(appdata->enemies,
                                              appdata->enemies);
     }
}

static Enemy *
_enemy_new(Evas_Object *parent, const char *str, Evas_Coord x, Evas_Coord y,
           Evas_Coord max_w, Evas_Coord max_h)
{
   assert(str);

   Enemy *enemy = calloc(1, sizeof(Enemy));
   if (!enemy) return NULL;

   Evas_Object *entry = elm_entry_add(parent);
   if (!entry)
     {
        free(enemy);
        return NULL;
     }
   elm_object_style_set(entry, "etypers");

   //To make layer order same to parent
   evas_object_smart_member_add(entry, parent);

   elm_entry_editable_set(entry, EINA_FALSE);
   elm_object_text_set(entry, str);

   //Prevent the text cutting out by the window bound
   //FIXME: TEXTBLOCK. Some words went outside of the textblock 
   Evas_Coord tb_w, tb_h;
   Evas_Object *tb = elm_entry_textblock_get(entry);
   evas_object_textblock_size_formatted_get(tb, &tb_w, &tb_h);
   x = (x + tb_w) > max_w ? (x - ((x + tb_w) - max_w)) : x;
   evas_object_move(entry, x, y);
   evas_object_resize(entry, tb_w, tb_h);
   evas_object_show(entry);

   enemy->entry = entry;
   enemy->x = x;
   enemy->y = y;
   enemy->w = tb_w;
   enemy->h = tb_h;

   return enemy;
}

static void
_enemy_add(AppData *appdata)
{
   unsigned int words_cnt = eina_array_count(appdata->words_array);
   char *word = eina_array_data_get(appdata->words_array,(rand() % words_cnt));
   Enemy *enemy = _enemy_new(appdata->bx, word,
                             (rand() % appdata->bound_w), 0, appdata->bound_w,
                             appdata->bound_h);
   appdata->enemies = eina_inlist_append(appdata->enemies,
                                         (Eina_Inlist *) enemy);
   appdata->interval_time = appdata->last_frame_time;
}

static void
_obj_del(void *data, Elm_Transit *transit)
{
   evas_object_del(data);
}

static void
_enemy_kill(AppData *appdata, Enemy *enemy)
{
   appdata->score += _enemy_score_get(enemy);

   Evas_Object *entry = enemy->entry;
   appdata->enemies = eina_inlist_remove(appdata->enemies,
                                         EINA_INLIST_GET(enemy));

   //Kill Effect
   Elm_Transit *transit = elm_transit_add();
   if (!transit) return;

   elm_entry_text_style_user_push(entry, "DEFAULT='color=#FF0000'");
   elm_transit_object_add(transit, entry);
   elm_transit_duration_set(transit, 0.25);
   elm_transit_effect_translation_add(transit, 0, 0, 15, -20);
   elm_transit_effect_color_add(transit, 255, 255, 255, 255, 0, 0, 0, 0);
   elm_transit_del_cb_set(transit, _obj_del, entry);
   elm_transit_go(transit);
}

static void
_enemy_explose(AppData *appdata, Enemy *enemy)
{
   appdata->score += _enemy_score_get(enemy);
   evas_object_del(enemy->entry);
   appdata->enemies = eina_inlist_remove(appdata->enemies,
                                         EINA_INLIST_GET(enemy));
   elm_object_signal_emit(appdata->ly, "elm,state,hit", "etypers");
}

static void
_update_gui(AppData *appdata)
{
   static char buf[64];

   //Level
   snprintf(buf, sizeof(buf), "%d", (int) appdata->level);
   elm_object_part_text_set(appdata->ly, "level_value", buf);

   //Score
   snprintf(buf, sizeof(buf), "%ld",
            ((unsigned long) ((appdata->level - 1) * 1000)) + appdata->score);
   elm_object_part_text_set(appdata->ly, "score_value", buf);
}

static void
_game_reset(AppData *appdata)
{
   elm_object_text_set(appdata->entry, "");
   elm_object_part_text_set(appdata->ly, "level_value", "1");
   elm_object_part_text_set(appdata->ly, "score_value", "0");
   elm_object_part_text_set(appdata->ly, "correct_value", "100%");

   _remove_all_enemies(appdata);
   _reset_defense_wall(appdata);
   elm_object_signal_emit(appdata->ly, "elm,state,gamereset", "etypers");

   appdata->score = 0;
   appdata->state = Ready;
   appdata->hit_cnt = 0.0f;
   appdata->hit_try_cnt = 0.0f;
   appdata->combo = 0;
}

static void
_game_over(AppData *appdata)
{
   ecore_animator_freeze(appdata->animator);
   elm_object_signal_emit(appdata->ly, "elm,state,gameover", "etypers");
   appdata->state = GameOver;
}

void _wall_hide(void *data, Elm_Transit *transit)
{
   evas_object_hide(data);
}

void _remove_wall(Evas_Object *wall)
{
   Elm_Transit *transit = elm_transit_add();
   if (!transit)
     {
        evas_object_hide(wall);
        return;
     }

   elm_transit_object_add(transit, wall);
   elm_transit_effect_color_add(transit, WALL_COLOR, 0, 0, 0, 0);
   elm_transit_del_cb_set(transit, _wall_hide, wall);
   elm_transit_duration_set(transit, 0.5);
   elm_transit_go(transit);
}

Eina_Bool _collision(AppData *appdata, Enemy *enemy)
{
   if (appdata->defense_rect.y > (Evas_Coord) (enemy->y + enemy->h))
     return EINA_FALSE;

   Eina_Bool collide = EINA_FALSE;

   Eina_List *l, *l_next;
   Evas_Object *wall;
   EINA_LIST_FOREACH_SAFE(appdata->defense_wall, l, l_next, wall)
     {
        Evas_Coord x, y, w, h;
        evas_object_geometry_get(wall, &x, &y, &w, &h);

        //Destroy walls collided with the enemy
        if (((enemy->x + enemy->w) >= x) && (enemy->x <= (x + w)) &&
            ((enemy->y + enemy->h) >= y) && (enemy->y <= (y + h)))
          {
             appdata->defense_wall =
                eina_list_remove_list(appdata->defense_wall, l);
             _remove_wall(wall);
             collide = EINA_TRUE;
          }
     }

   return collide;
}

static Eina_Bool
_animator_cb(void *data)
{
//FIXME: Sometimes this func is called abnormally. Need to check.
   AppData *appdata = data;
   double current_time = ecore_time_get();
   double elapsed_time = current_time - appdata->interval_time;

   //Update Enemies
   Enemy *enemy;
   Eina_Inlist *l;
   float drop_dist = (float) ((current_time - appdata->last_frame_time) *
                              DROP_DIST_WEIGHT) + (appdata->level * 0.095);

   EINA_INLIST_FOREACH_SAFE(appdata->enemies, l, enemy)
     {
        enemy->y += drop_dist;

        //Reached to the bottom... Boom!
        if (enemy->y > appdata->bound_h)
          {
             _game_over(appdata);
             return ECORE_CALLBACK_RENEW;
          }

        //Collide with defense wall
        if (_collision(appdata, enemy))
          {
             _enemy_explose(appdata, enemy);
             continue;
          }

        evas_object_move(enemy->entry, (Evas_Coord) enemy->x,
                         (Evas_Coord) enemy->y);
     }

   appdata->level += ((current_time - appdata->last_frame_time) *
                      LEVEL_INC_WEIGHT);
   appdata->last_frame_time = current_time;

   //Add Enemy?
   if ((elapsed_time + level_handicap_for_enemy_add(appdata->level)) >
       (NEW_ENEMY_DURATION_SEC + appdata->level))
     _enemy_add(appdata);

   _update_gui(appdata);

   return ECORE_CALLBACK_RENEW;
}

static void
_app_release(AppData *appdata)
{
   ecore_animator_del(appdata->animator);

   _remove_all_enemies(appdata);

   //Remove all words allocated
   while (eina_array_count(appdata->words_array))
     free(eina_array_pop(appdata->words_array));
   eina_array_free(appdata->words_array);

   eina_list_free(appdata->defense_wall);

   free(appdata);

   elm_exit();
}

static void
_win_del(void *data, Evas_Object *obj, void *event_info)
{
   _app_release(data);
}

static void
_win_resize(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   AppData *appdata = data;
   Evas_Coord w, h;
   evas_object_geometry_get(obj, NULL, NULL, &w, &h);
   appdata->bound_w = w;
   appdata->bound_h = h;
}

static Evas_Object *
_layout_create(Evas_Object *win, const char *filepath, const char *group)
{
   Evas_Object *ly = elm_layout_add(win);
   elm_layout_file_set(ly, filepath, group);
   evas_object_size_hint_weight_set(ly, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, ly);
   evas_object_show(ly);

   return ly;
}

static Evas_Object *
_box_create(Evas_Object *ly, const char *part)
{
   Evas_Object *bx = elm_box_add(ly);
   evas_object_show(bx);
   elm_object_part_content_set(ly, part, bx);

   return bx;
}

static void
_combo(AppData *appdata, Enemy *enemy, int combo)
{
   appdata->score += _enemy_score_get(enemy) * combo;

   Evas *e = evas_object_evas_get(appdata->win);
   Evas_Object *text = evas_object_text_add(e);
   if (!text) return;

   evas_object_text_style_set(text, EVAS_TEXT_STYLE_PLAIN);
   evas_object_text_font_set(text, "Sans", 13);

   char buf[12];
   snprintf(buf, sizeof(buf), "%dx Combo!!", combo);
   evas_object_text_text_set(text, buf);

   Evas_Coord w, h;
   evas_object_geometry_get(enemy->entry, NULL, NULL, &w, &h);
   evas_object_move(text, (int) enemy->x, (int) enemy->y);
   evas_object_resize(text, 100, 20);
   evas_object_color_set(text, 255, 255, 0, 255);
   evas_object_show(text);

   //Combo Text Effect
   Elm_Transit *transit = elm_transit_add();
   if (!transit)
     {
        evas_object_del(text);
        return;
     }

   elm_transit_object_add(transit, text);
   elm_transit_effect_zoom_add(transit, 1, 1.25);
   elm_transit_effect_color_add(transit, 255, 255, 0, 255, 0, 0, 0, 0);
   elm_transit_del_cb_set(transit, _obj_del, text);
   elm_transit_duration_set(transit, 1);
   elm_transit_go(transit);
}

static void _minus_score(AppData *appdata)
{
   appdata->score -= MINUS_SCORE;
   if (appdata->score < 0)
     appdata->score = 0;
}

static void
_update_correctness(AppData *appdata, Eina_Bool hit)
{
   appdata->hit_try_cnt += 1.0f;
   if (hit) appdata->hit_cnt += 1.0f;

   char buf[10];
   int correctness = (int) ((appdata->hit_cnt / appdata->hit_try_cnt) * 100.0f);
   snprintf(buf, sizeof(buf), "%3d%%", correctness);
   elm_object_part_text_set(appdata->ly, "correct_value", buf);
}

static void
_enemies_kill(AppData *appdata, Evas_Object *obj, const char *input_text,
              Eina_Bool space)
{
   //Compare the enemies and clear them.
   Eina_Inlist *l;
   Enemy *enemy;
   const char *enemy_text;
   Eina_Bool hit = EINA_FALSE;

   EINA_INLIST_FOREACH_SAFE(appdata->enemies, l, enemy)
     {
        enemy_text = elm_object_text_get(enemy->entry);
        if (!enemy_text) continue;
        if (space)
          {
             if (strlen(enemy_text) != (strlen(input_text) - 1)) continue;
          }
        else
          {
             if (strlen(enemy_text) != strlen(input_text)) continue;
          }

        if (!strncmp(input_text, enemy_text, (strlen(input_text) - 1)))
          {
             hit = EINA_TRUE;
             appdata->combo++;
             if (appdata->combo > 1)
               _combo(appdata, enemy, appdata->combo);
             _enemy_kill(appdata, enemy);
          }
     }

   //Minus score if it doesn't any hits
   if (!hit)
     {
        _minus_score(appdata);
        appdata->combo = 0;
     }

   _update_correctness(appdata, hit);

   elm_object_text_set(obj, NULL);

}

static void
_changed_user_cb(void *data, Evas_Object *obj, void *event_info)
{
   const char *input_text = elm_object_text_get(obj);
   if (!input_text) return;

   //try to kill the enemies when user press the "space" key
   if (input_text[strlen(input_text) -1] != ' ') return;

   _enemies_kill(data, obj, input_text, EINA_TRUE);
}

static void
_game_level_cb(void *data, Evas_Object *obj, void *event_info)
{
   AppData *appdata = evas_object_data_get(obj, "appdata");
   evas_object_del(obj);
   appdata->level = (int) data;

   _game_reset(appdata);
   appdata->state = Playing;
   _resume(appdata);
}

static void
_level_back_btn_cb(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *popup = data;
   AppData *appdata = evas_object_data_get(popup, "appdata");
   appdata->state = appdata->prev_state;
   evas_object_del(popup);
   _pause(appdata);
   _popup(appdata);
}

static void
_game_start_cb(void *data, Evas_Object *obj, void *event_info)
{
   AppData *appdata = data;
   evas_object_del(appdata->popup);
   appdata->popup = NULL;
   appdata->prev_state = appdata->state;
   appdata->state = SubPopup;
   Evas_Object *popup = elm_popup_add(appdata->win);
   elm_object_style_set(popup, "etypers");
   evas_object_data_set(popup, "appdata", data);
   elm_object_part_text_set(popup, "title,text", "Start Level");
   //FIXME: Content height is not fit to the actual total item height
   elm_popup_item_append(popup, "Level 1", NULL, _game_level_cb, (void *) 1);
   elm_popup_item_append(popup, "Level 2", NULL, _game_level_cb, (void *) 2);
   elm_popup_item_append(popup, "Level 3", NULL, _game_level_cb, (void *) 3);
   elm_popup_item_append(popup, "Level 4", NULL, _game_level_cb, (void *) 4);
   elm_popup_item_append(popup, "Level 5", NULL, _game_level_cb, (void *) 5);
   elm_popup_item_append(popup, "Level 6", NULL, _game_level_cb, (void *) 6);
   elm_popup_item_append(popup, "Level 7", NULL, _game_level_cb, (void *) 7);
   elm_popup_item_append(popup, "Level 8", NULL, _game_level_cb, (void *) 8);
   elm_popup_item_append(popup, "Level 9", NULL, _game_level_cb, (void *) 9);

   //FIXME: It doesn't work
   elm_object_scroll_hold_push(popup);
   evas_object_show(popup);

   Evas_Object *btn = elm_button_add(popup);
   if (!btn) return;
   elm_object_text_set(btn, "Back");
   elm_object_part_content_set(popup, "button1", btn);
   evas_object_smart_callback_add(btn, "clicked", _level_back_btn_cb, popup);
}

static void
_top_ranking_cb(void *data, Evas_Object *obj, void *event_info)
{
  //TODO: 
}

static void
_game_exit_cb(void *data, Evas_Object *obj, void *event_info)
{
   _app_release(data);
}

static void
_credit_back_btn_cb(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *popup = data;
   AppData *appdata = evas_object_data_get(popup, "appdata");
   appdata->state = appdata->prev_state;
   evas_object_show(appdata->popup);
   evas_object_del(popup);
}

static void
_credit_cb(void *data, Evas_Object *obj, void *event_info)
{
   AppData *appdata = data;

   Evas_Object *popup = elm_popup_add(appdata->win);
   if (!popup) return;
   appdata->state = SubPopup;
   elm_object_style_set(popup, "etypers");
   elm_object_part_text_set(popup, "title,text", "Game Credits");
   elm_object_text_set(popup,
                       "Enlightenment Typers v1.0<br>Designed by Hermet");
   evas_object_data_set(popup, "appdata", appdata);

   Evas_Object *btn = elm_button_add(popup);
   if (!btn)
     {
        evas_object_del(popup);
        return;
     }

   elm_object_text_set(btn, "Back");
   elm_object_part_content_set(popup, "button1", btn);
   evas_object_smart_callback_add(btn, "clicked", _credit_back_btn_cb, popup);
   evas_object_show(popup);

   evas_object_hide(appdata->popup);
}

static void
_option_cb(void *data, Evas_Object *obj, void *event_info)
{
   //TODO:
}

static Evas_Object *
_popup_create(Evas_Object *parent, AppData *appdata)
{
   Evas_Object *popup = elm_popup_add(parent);
   elm_object_style_set(popup, "etypers");
   elm_object_part_text_set(popup, "title,text", "E-Typers Menu");
   elm_popup_item_append(popup, "Start", NULL, _game_start_cb,
                              appdata);
//   Elm_Object_Item *it;
//   it = elm_popup_item_append(popup, "Option", NULL, _option_cb, NULL);
//   elm_object_item_disabled_set(it, EINA_TRUE);
//   it = elm_popup_item_append(popup, "Ranking", NULL, _top_ranking_cb, NULL);
//   elm_object_item_disabled_set(it, EINA_TRUE);
   elm_popup_item_append(popup, "Credits", NULL, _credit_cb, appdata);

   elm_popup_item_append(popup, "Exit", NULL, _game_exit_cb, appdata);
   //FIXME: It doesn't work
   elm_object_scroll_hold_push(popup);
   evas_object_show(popup);
   return popup;
}

static void
_popup(AppData *appdata)
{
   if (!appdata->popup)
     {
        Evas_Object *popup = _popup_create(appdata->win, appdata);
        appdata->popup = popup;
     }
   else
     {
        evas_object_del(appdata->popup);
        appdata->popup = NULL;
     }
}

static void
_pause(AppData *appdata)
{
   elm_object_disabled_set(appdata->entry, EINA_TRUE);
   ecore_animator_freeze(appdata->animator);
}

static void
_resume(AppData *appdata)
{
   elm_object_disabled_set(appdata->entry, EINA_FALSE);
   elm_object_focus_set(appdata->entry, EINA_TRUE);
   appdata->last_frame_time = ecore_time_get();
   ecore_animator_thaw(appdata->animator);
}

static void
_pause_or_resume(AppData *appdata)
{
   if (appdata->state == Paused)
     {
        appdata->state = Playing;
        _resume(appdata);
     }
   else if (appdata->state == Playing)
     {
        appdata->state = Paused;
        _pause(appdata);
     }
   _popup(appdata);
}

static Eina_Bool
_key_down_cb(void *data, int type, void *event_info)
{
   Ecore_Event_Key *event = event_info;
   AppData *appdata = data;

   if ((appdata->state == Ready) || (appdata->state == SubPopup))
     return ECORE_CALLBACK_PASS_ON;

   //When input is "return" as well as "space" then it tries to kill the enemies
   //also.
   if (!strcmp(event->keyname, "Return"))
     {
        if (appdata->state != GameOver)
          {
             const char *input_text = elm_object_text_get(appdata->entry);
             if (input_text)
               _enemies_kill(appdata, appdata->entry, input_text, EINA_FALSE);
          }
     }
   //Pause/Resume
   else if (!strcmp(event->keyname, "Escape"))
     {
        if (appdata->state == GameOver)
          _game_reset(appdata);
        _pause_or_resume(appdata);
     }

   return ECORE_CALLBACK_PASS_ON;
}

static Evas_Object *
_entry_create(Evas_Object *ly, const char *part, AppData *appdata)
{
   Evas_Object *entry = elm_entry_add(ly);
   elm_object_style_set(entry, "etypers");
   elm_entry_single_line_set(entry, EINA_TRUE);
   elm_entry_input_panel_enabled_set(entry, EINA_FALSE);
   evas_object_show(entry);
   elm_object_part_content_set(ly, part, entry);
   evas_object_smart_callback_add(entry, "changed,user",  _changed_user_cb,
                                  appdata);
   return entry;
}

static Evas_Object *
_win_create(int w, int h, AppData *appdata)
{
   Evas_Object *win = elm_win_util_standard_add("Enlightenment Typers",
                                                "Enlightenment Typers v1.0");
   elm_win_autodel_set(win, EINA_TRUE);
   evas_object_smart_callback_add(win, "delete,request", _win_del, NULL);
   evas_object_resize(win, w, h);
   evas_object_show(win);
   evas_object_event_callback_add(appdata->win, EVAS_CALLBACK_RESIZE,
                                  _win_resize, appdata);
   return win;
}

static Eina_Array *
_words_create()
{
   Eina_Array *array = eina_array_new(400);
   if (!array) return NULL;

   char buf[128];
   FILE *fp = fopen("./words.txt", "r");
   if (!fp)
     {
        eina_array_free(array);
        return NULL;
     }

   while(0 < fscanf(fp, "%s", buf))
     eina_array_push(array, strdup(buf));

   return array;
}

static void
_table_resize_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Evas_Coord x, y, w, h;
   evas_object_geometry_get(obj, &x, &y, &w, &h);

   AppData *appdata = data;
   appdata->defense_rect.x = x;
   appdata->defense_rect.y = y;
   appdata->defense_rect.w = w;
   appdata->defense_rect.h = h;
}

static Evas_Object *
_table_create(Evas_Object *ly, const char *part, AppData *appdata)
{
   Evas_Object *table = elm_table_add(ly);
   if (!table) return NULL;
   elm_table_homogeneous_set(table, EINA_TRUE);
   elm_table_padding_set(table, 1, 1);
   elm_object_part_content_set(ly, "defense_wall", table);
   evas_object_show(table);
   evas_object_event_callback_add(table, EVAS_CALLBACK_RESIZE, _table_resize_cb,
                                  appdata);
   return table;
}

static Eina_List *
_defense_wall_set(Evas_Object *table)
{
   Evas *evas = evas_object_evas_get(table);
   Eina_List *list = NULL;
   int i, j;

   for (i = 0; i < DEFENSE_WALL_CNT; i++)
     {
        for (j = 0; j < 2; j++)
          {
             Evas_Object *obj = evas_object_rectangle_add(evas);
             if (!obj) continue;
             evas_object_color_set(obj, WALL_COLOR);
             evas_object_size_hint_align_set(obj, EVAS_HINT_FILL,
                                             EVAS_HINT_FILL);
             evas_object_size_hint_weight_set(obj, EVAS_HINT_EXPAND,
                                              EVAS_HINT_EXPAND);
             evas_object_show(obj);
             elm_table_pack(table, obj, i, j, 1, 1);

             list = eina_list_append(list, obj);
          }
     }
   return list;
}

static void
_reset_defense_wall(AppData *appdata)
{
   eina_list_free(appdata->defense_wall);
   evas_object_del(appdata->table);
   appdata->table = _table_create(appdata->ly, "table", appdata);
   appdata->defense_wall = _defense_wall_set(appdata->table);
}

static void
_app_init(AppData *appdata)
{
   appdata->words_array = _words_create();
   appdata->win = _win_create(DEFAULT_WIN_W, DEFAULT_WIN_H, appdata);
   appdata->ly = _layout_create(appdata->win, "./etypers.edj", "gui");
   appdata->bx = _box_create(appdata->ly, "enemies");
   appdata->entry = _entry_create(appdata->ly, "entry", appdata);
   appdata->table = _table_create(appdata->ly, "table", appdata);
   appdata->defense_wall = _defense_wall_set(appdata->table);
   appdata->state = Ready;
   appdata->level = 1;
   appdata->bound_w = DEFAULT_WIN_W;
   appdata->bound_h = DEFAULT_WIN_H;
   appdata->animator = ecore_animator_add(_animator_cb, appdata);
}

int
elm_main(int argc, char **argv)
{
   AppData *appdata = calloc(1, sizeof(AppData));
   if (!appdata) return 0;

   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);
   elm_theme_extension_add(NULL, "./etypers.edj");
   srand((unsigned int) time(NULL));
   ecore_event_handler_add(ECORE_EVENT_KEY_DOWN, _key_down_cb, appdata);

   _app_init(appdata);
   _pause(appdata);
   _popup(appdata);

   elm_run();
   elm_shutdown();

   return 0;
}
ELM_MAIN()
