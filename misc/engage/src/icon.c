#include "engage.h"
#include "limits.h"
#include "config.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#ifdef HAVE_IMLIB
#include <Imlib2.h>
#endif
// for stat
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#ifdef DMALLOC
#include "dmalloc.h"
#endif
#include <assert.h>

Evas_List      *icon_mappings = NULL;
Evas_List      *icon_paths = NULL;

static OD_Icon *od_icon_new(const char *winclass, const char *name,
                            const char *icon_path);
static void     od_icon_mapping_get(const char *winclass, char **name, char **icon_name);       // DON'T free returned
static char    *od_icon_path_get(const char *icon_name);
static void     od_icon_edje_app_cb(void *data, Evas_Object * obj,
                                    const char *emission, const char *source);
static void     od_icon_edje_win_minimize_cb(void *data, Evas_Object * obj,
                                             const char *emission,
                                             const char *source);
static void     od_icon_edje_win_raise_cb(void *data, Evas_Object * obj,
                                          const char *emission,
                                          const char *source);

OD_Icon        *
od_icon_new_applnk(const char *command, const char *winclass)
{
  OD_Icon        *ret = NULL;
  char           *name = NULL;
  char           *icon_name = NULL;
  char           *icon_path = NULL;

  od_icon_mapping_get(winclass, &name, &icon_name);
  icon_path = od_icon_path_get(icon_name);
  ret = od_icon_new(winclass, name, icon_path);

#if 0
  fprintf(stderr,
          "new applnk: name=\"%s\" winclass=\"%s\" icon_path=\"%s\" command=\"%s\"\n",
          name, winclass, icon_path, command);
#endif
  ret->type = application_link;
  ret->data.applnk.command = strdup(command);
  ret->data.applnk.winclass = strdup(winclass);
  ret->data.applnk.count = 0;
  free(icon_path);
  return ret;
}

OD_Icon        *
od_icon_new_dicon(const char *command, const char *name, const char *icon_name)
{
  char           *icon_path = od_icon_path_get(icon_name);
  OD_Icon        *ret = od_icon_new(name, name, icon_path);

#if 0
  fprintf(stderr, "new dicon: name=\"%s\" icon_path=\"%s\" command=\"%s\"\n",
          name, icon_path, command);
#endif
  ret->type = docked_icon;
  ret->data.applnk.command = strdup(command);
  free(icon_path);
  return ret;
}

OD_Icon        *
od_icon_new_minwin(Ecore_X_Window win)
{
  OD_Icon        *ret = NULL;
  char           *icon_path = NULL;
  char           *name = NULL, *icon_name = NULL;
  char           *winclass = NULL;
  char           *title = NULL;

  winclass = od_wm_get_winclass(win);
  title = od_wm_get_title(win);
  od_icon_mapping_get(winclass, &name, &icon_name);
  icon_path = od_icon_path_get(icon_name);

  if ((ret = od_icon_new(winclass, title, icon_path))) {
#ifdef HAVE_IMLIB
    if (options.grab_min_icons != 0)
      od_icon_grab(ret, win);
#endif
    ret->type = minimised_window;
    ret->data.minwin.window = win;
  }
#if 0
  fprintf(stderr, "new minwin: icon_path=\"%s\"\n", icon_path);
#endif
  if (winclass)
    free(winclass);
  if (title)
    free(title);
  if (icon_path)
    free(icon_path);
  return ret;
}

#ifdef HAVE_IMLIB
void
od_icon_grab(OD_Icon * icon, Ecore_X_Window win)
{
  XWMHints       *hints = NULL;
  Imlib_Image     img = NULL;
  Display        *dsp = NULL;
  int             scr = 0, x = 0, y = 0, w = 0, h = 0;
  Pixmap          pmap, mask;
  Evas_Object    *obj = NULL;

  assert(icon);
  if (icon->pic && !strcmp(evas_object_type_get(icon->pic), "edje"))
    return;

  dsp = ecore_x_display_get();
  scr = DefaultScreen(dsp);

  hints = XGetWMHints(dsp, win);
  if (hints == NULL)
    goto done;
  pmap = (hints->flags & IconPixmapHint) ? hints->icon_pixmap : None;
  mask = (hints->flags & IconMaskHint) ? hints->icon_mask : None;
  XFree(hints);
  if (pmap == None)
    goto done;

  ecore_x_pixmap_geometry_get(pmap, &x, &y, &w, &h);
  imlib_context_set_display(dsp);
  imlib_context_set_visual(DefaultVisual(dsp, scr));
  imlib_context_set_colormap(None);
  imlib_context_set_dither_mask(0);
  imlib_context_set_drawable(pmap);

/* include the if'd out block if you want to use imlib version < 1.1.1 */
#if 0
  mask = None;
#endif
  img = imlib_create_image_from_drawable(mask, x, y, w, h, 0);
  imlib_context_set_image(img);

#if 0
  /* This will only be necessary if we move to non image objects as pics */
  if ((obj = edje_object_part_swallow_get(icon->icon, "EngageIcon"))) {
    edje_object_part_unswallow(icon->icon, obj);
    evas_object_del(obj);
  }
#endif
  evas_object_image_size_set(icon->pic, w, h);
  evas_object_image_data_copy_set(icon->pic,
                                  imlib_image_get_data_for_reading_only());
  edje_object_part_swallow(icon->icon, "EngageIcon", icon->pic);

  imlib_free_image();

done:
  return;                       // just fix compiler warnings - why do we have an empty done?
}
#endif

void
od_object_resize_intercept_cb(void *data, Evas_Object * o,
                              Evas_Coord w, Evas_Coord h)
{
  if (o) {
    if (!strcmp("image", evas_object_type_get(o))) {
      evas_object_image_fill_set(o, 0.0, 0.0, w, h);
    } else {
#if 0
      fprintf(stderr, "Intercepting something other than an image(%s)\n",
              evas_object_type_get(o));
#endif
    }
  }
  evas_object_resize(o, w, h);
}

void
od_icon_reload(OD_Icon * in)
{
  const char     *icon_part = NULL;
  char           *path, *winclass, *name, *icon_file;

  Evas_Object    *icon = NULL;
  Evas_Object    *pic = NULL;
  Evas_Object    *tt_txt = NULL;
  Evas_Object    *tt_shd = NULL;

  icon = in->icon;
  winclass = in->winclass;
  name = in->name;
  icon_file = in->icon_file;
  pic = in->pic;

  path = ecore_config_theme_with_path_from_name_get(options.theme);
  if (edje_object_file_set(icon, path, "Main") > 0) {
#if 0
    fprintf(stderr, "Trying to find part for %s\n", winclass);
#endif
    if ((icon_part = edje_object_data_get(icon, winclass))) {
      pic = edje_object_add(evas_object_evas_get(icon));
      if (edje_object_file_set(pic, path, icon_part) > 0) {
#if 0
        fprintf(stderr, "Found icon part for %s(%s)\n", name, icon_part);
#endif
      } else if (edje_object_file_set(pic, path, "Unknown") > 0) {
#if 0
        fprintf(stderr, "Didn't Find icon part for %s\n", name);
#endif
      } else {
        evas_object_del(pic);
        pic = NULL;
      }
    } else {
      pic = edje_object_add(evas_object_evas_get(icon));
      if (edje_object_file_set(pic, path, "Unknown") > 0) {
#if 0
        fprintf(stderr, "Didn't Find icon part for %s\n", name);
#endif
      } else {
        evas_object_del(pic);
        pic = NULL;
      }
    }
    if (!pic) {
      pic = evas_object_image_add(evas);
      evas_object_image_file_set(pic, icon_file, NULL);
      evas_object_image_alpha_set(pic, 1);
      evas_object_image_smooth_scale_set(pic, 1);
      evas_object_pass_events_set(pic, 1);
      evas_object_intercept_resize_callback_add(pic,
                                                od_object_resize_intercept_cb,
                                                NULL);
    }

    in->pic = pic;
    evas_object_layer_set(pic, 100);
    evas_object_move(pic, -50, -50);
    evas_object_resize(pic, 32, 32);
    evas_object_show(pic);
    if (edje_object_part_exists(icon, "EngageIcon")) {
      edje_object_part_swallow(icon, "EngageIcon", pic);
    } else {
      evas_object_del(pic);
      in->pic = NULL;
    }
    if (edje_object_part_exists(icon, "EngageName")) {
      edje_object_part_text_set(icon, "EngageName", name);
    } else {
      tt_txt = in->tt_txt = evas_object_text_add(evas);
      tt_shd = in->tt_shd = evas_object_text_add(evas);
      evas_object_text_font_set(tt_txt, options.tt_fa, options.tt_fs);
      evas_object_text_text_set(tt_txt, name);
      evas_object_color_set(tt_txt,
                            (options.tt_txt_color >> 16) & 0xff,
                            (options.tt_txt_color >> 8) & 0xff,
                            (options.tt_txt_color >> 0) & 0xff, 255);
      evas_object_layer_set(tt_txt, 200);

      evas_object_text_font_set(tt_shd, options.tt_fa, options.tt_fs);
      evas_object_text_text_set(tt_shd, name);
      evas_object_color_set(tt_shd,
                            (options.tt_shd_color >> 16) & 0xff,
                            (options.tt_shd_color >> 8) & 0xff,
                            (options.tt_shd_color >> 0) & 0xff, 127);
      evas_object_layer_set(tt_shd, 199);
    }
    edje_object_signal_callback_add(icon, "engage,app,*", "*",
                                    od_icon_edje_app_cb, in);
    edje_object_signal_callback_add(icon, "engage,window,minimize*", "*",
                                    od_icon_edje_win_minimize_cb, in);
    edje_object_signal_callback_add(icon, "engage,window,raise*", "*",
                                    od_icon_edje_win_raise_cb, in);
    evas_object_layer_set(icon, 100);
    evas_object_show(icon);
  } else {
    evas_object_del(icon);
    in->icon = NULL;
  }
  free(path);

#ifdef HAVE_IMLIB
  if (options.grab_min_icons != 0)
    od_icon_grab(in, in->data.minwin.window);
#endif

}

OD_Icon        *
od_icon_new(const char *winclass, const char *name, const char *icon_file)
{
  OD_Icon        *ret = NULL;

  assert(winclass);
  assert(name);
  assert(icon_file);

  ret = (OD_Icon *) malloc(sizeof(OD_Icon));
  memset(ret, 0, sizeof(OD_Icon));
  ret->winclass = strdup(winclass);
  ret->name = strdup(name);
  ret->icon_file = strdup(icon_file);
  ret->icon = edje_object_add(evas);

  od_icon_reload(ret);

  return ret;
}

void
od_icon_del(OD_Icon * icon)
{
  assert(icon);
  switch (icon->type) {
  case application_link:
    assert(icon->data.applnk.command);
    assert(icon->data.applnk.winclass);
    free(icon->data.applnk.command);
    free(icon->data.applnk.winclass);
    break;
  case docked_icon:
    assert(icon->data.applnk.command);
    free(icon->data.dicon.command);
    break;
  case minimised_window:
    break;
  }

  assert(icon->icon);
  evas_object_del(icon->icon);
  if (icon->pic)
    evas_object_del(icon->pic);
  if (icon->tt_txt)
    evas_object_del(icon->tt_txt);
  if (icon->tt_shd)
    evas_object_del(icon->tt_shd);
  if (icon->arrow)
    evas_object_del(icon->arrow);
  assert(icon->name);
  free(icon->name);
  free(icon);
}

void
od_icon_arrow_show(OD_Icon * icon)
{
  assert(icon);
  assert(icon->icon);
  edje_object_signal_emit(icon->icon, "engage,app,opened", "");
#if 0
  if (icon->arrow)
    evas_object_show(icon->arrow);
  else {
    icon->arrow = evas_object_image_add(evas);
    int             height = (int) options.arrow_size;
    int             width = 1 + 2 * (int) options.arrow_size;
    int            *pattern = (int *) malloc(sizeof(int) * width * height);

    int             x, y;

    for (y = 0; y < height; y++) {
      for (x = 0; x < width; x++)
        pattern[y * width + x] = (x + y < 3 * height / 2 ||
                                  y - x <
                                  -height / 2 ? 0x00000000 : 0xff000000);
    }

    evas_object_image_alpha_set(icon->arrow, 1);
    evas_object_image_size_set(icon->arrow, width, height);
    evas_object_image_smooth_scale_set(icon->arrow, 0);
    evas_object_image_data_copy_set(icon->arrow, pattern);
    evas_object_image_data_update_add(icon->arrow, 0, 0, width, height);
    evas_object_image_fill_set(icon->arrow, 0.0, 0.0, width, height);
    evas_object_resize(icon->arrow, width, height);
    evas_object_layer_set(icon->arrow, 100);
    evas_object_show(icon->arrow);
    free(pattern);
  }
#endif
}

void
od_icon_arrow_hide(OD_Icon * icon)
{
  assert(icon);
  assert(icon->icon);
  edje_object_signal_emit(icon->icon, "engage,app,closed", "");
#if 0
  if (icon->arrow)
    evas_object_hide(icon->arrow);
#endif
}

void
od_icon_tt_show(OD_Icon * icon)
{
  if (icon->tt_txt)
    evas_object_show(icon->tt_txt);
  if (icon->tt_shd)
    evas_object_show(icon->tt_shd);
}

void
od_icon_tt_hide(OD_Icon * icon)
{
  if (icon->tt_txt)
    evas_object_hide(icon->tt_txt);
  if (icon->tt_shd)
    evas_object_hide(icon->tt_shd);
}

void
od_icon_name_change(OD_Icon * icon, const char *name)
{
  free(icon->name);
  icon->name = strdup(name);
  evas_object_text_text_set(icon->tt_txt, name);
  evas_object_text_text_set(icon->tt_shd, name);
  need_redraw = true;
}

void
od_icon_mapping_add(const char *winclass, const char *name,
                    const char *icon_name)
{
  icon_mappings = evas_list_append(icon_mappings, strdup(winclass));
  icon_mappings = evas_list_append(icon_mappings, strdup(name));
  icon_mappings = evas_list_append(icon_mappings, strdup(icon_name));
}

void
od_icon_mapping_get(const char *winclass, char **name, char **icon_name)
{
  Evas_List      *item = icon_mappings;

#if 0
  printf("getting mapping for %s\n", winclass);
#endif
  while (item) {
    if (strcmp(winclass, (char *) item->data) == 0) {
      if (item->next) {
        *name = (char *) item->next->data;
        if (item->next->next) {
          *icon_name = (char *) item->next->next->data;
        } else {
          fprintf(stderr, "corrupt icon mapping for %s (icon_name)\n",
                  winclass);
        }
      } else {
        fprintf(stderr, "corrupt icon mapping for %s (name)\n", winclass);
      }
      return;
    }
    item = item->next;
  }
  *name = strdup(winclass);
  *icon_name = strdup(winclass);
}

char           *
od_icon_path_get(const char *icon_name)
{
  Evas_List      *item = icon_paths;

  while (item) {
    char           *path = (char *) item->data;
    char            buffer[strlen(path) + strlen(icon_name) + strlen(".png") + 2];      // one extra for '/', another for '\0'
    struct stat     dummy;

    strcpy(buffer, path);
    strcat(buffer, "/");
    strcat(buffer, icon_name);
    strcat(buffer, ".png\0");
    if (stat(buffer, &dummy) == 0)
      return strdup(buffer);
    item = item->next;
  }

  if (strcmp(icon_name, "xapp") != 0)
    return od_icon_path_get("xapp");
  else
    return NULL;
}

void
od_icon_add_path(const char *path)
{
  icon_paths = evas_list_append(icon_paths, strdup(path));
}

void
od_icon_add_kde_set(const char *path)
{
  static char    *sizes[] =
    { "128x128", "96x96", "64x64", "48x48", "32x32", "24x24", "16x16", "" };
  static char    *types[] = { "apps", "devices", "filesystems", "actions", "" };

  char          **size = sizes;

  while ((*size)[0] != '\0') {
    char          **type = types;

    while ((*type)[0] != '\0') {
      char            buffer[PATH_MAX];
      struct stat     dummy;

      strcpy(buffer, path);
      strcat(buffer, "/");
      strcat(buffer, *size);
      strcat(buffer, "/");
      strcat(buffer, *type);
      if (stat(buffer, &dummy) == 0)
        od_icon_add_path(buffer);
      type++;
    }
    size++;
  }
}
static void
od_icon_edje_app_cb(void *data, Evas_Object * obj, const char *emission, const
                    char *source)
{
  pid_t           pid;
  Evas_List      *l = NULL;
  const char     *winclass = NULL;
  OD_Icon        *icon = NULL;
  OD_Window      *win = NULL;

  if ((icon = (OD_Icon *) data)) {
    if (!strcmp(emission, "engage,app,open")) {
      switch (icon->type) {
      case application_link:
      case docked_icon:
        edje_object_signal_emit(obj, "engage,app,open,ok", "");
        ecore_exe_run(icon->data.applnk.command, NULL);
        break;
      case minimised_window:
        break;
      }
    } else if (!strcmp(emission, "engage,app,close")) {
      if (icon->data.applnk.winclass) {
        if ((winclass = icon->data.applnk.winclass)) {
          win = od_wm_window_current_by_window_class_get(winclass);
          ecore_x_window_prop_protocol_set(win->id,
                                           ECORE_X_WM_PROTOCOL_DELETE_REQUEST,
                                           1);
        }
      }
    }
    fprintf(stderr, "App got %s from %s\n", emission, icon->name);
  }
}
static void
od_icon_edje_win_minimize_cb(void *data, Evas_Object * obj,
                             const char *emission, const
                             char *source)
{
  pid_t           pid;
  Evas_List      *l = NULL;
  OD_Icon        *icon = NULL;

  if ((icon = (OD_Icon *) data)) {
    if (!strcmp(emission, "engage,window,minimize")) {
      switch (icon->type) {
      case application_link:
        for (l = clients; l; l = l->next) {
          OD_Window      *win = (OD_Window *) l->data;

          if (win->applnk == icon && !win->minwin) {
            od_wm_deactivate_window(win->id);
            break;
          }
        }
        break;
      case docked_icon:
        break;
      case minimised_window:
        break;
      }
    } else if (!strcmp(emission, "engage,window,minimize,all")) {
      switch (icon->type) {
      case application_link:
      case docked_icon:
      case minimised_window:
        for (l = clients; l; l = l->next) {
          OD_Window      *win = (OD_Window *) l->data;

          if (win->applnk == icon && !win->minwin) {
            od_wm_deactivate_window(win->id);
          }
        }
        break;
      default:
        break;
      }
    }
#if 0
    fprintf(stderr, "Minimize got %s from %s\n", emission, icon->name);
#endif
  }
}
static void
od_icon_edje_win_raise_cb(void *data, Evas_Object * obj, const char *emission, const
                          char *source)
{
  const char     *winclass = NULL;
  Evas_List      *l = NULL;
  OD_Icon        *icon = NULL;
  OD_Window      *win = NULL;

  if ((icon = (OD_Icon *) data)) {
    if (!strcmp(emission, "engage,window,raise")) {
      switch (icon->type) {
      case application_link:
      case docked_icon:
      case minimised_window:
        for (l = clients; l; l = l->next) {
          win = (OD_Window *) l->data;

          if (win->minwin == icon || win->applnk == icon) {
            od_wm_activate_window(win->id);
            break;
          }
        }
        break;
      default:
        break;
      }
    } else if (!strcmp(emission, "engage,window,raise,all")) {
      for (l = clients; l; l = l->next) {
        win = (OD_Window *) l->data;

        if (win->minwin == icon || win->applnk == icon) {
          od_wm_activate_window(win->id);
          ecore_x_window_prop_state_request(win->id,
                                            ECORE_X_WINDOW_STATE_ICONIFIED, 0);
        }
      }
    } else if (!strcmp(emission, "engage,window,raise,next")) {
      switch (icon->type) {
      case application_link:
      case docked_icon:
        if (icon->data.applnk.winclass) {
          winclass = icon->data.applnk.winclass;
          if ((win = od_wm_window_next_by_window_class_get(winclass))) {
            od_wm_activate_window(win->id);
          }
        }
        break;
      case minimised_window:
        break;
      default:
        break;
      }
    } else if (!strcmp(emission, "engage,window,raise,prev")) {
      switch (icon->type) {
      case application_link:
      case docked_icon:
        if (icon->data.applnk.winclass) {
          winclass = icon->data.applnk.winclass;
          if ((win = od_wm_window_prev_by_window_class_get(winclass))) {
            od_wm_activate_window(win->id);
          }
        }
        break;
      case minimised_window:
        break;
      default:
        break;
      }
    }
#if 0
    fprintf(stderr, "Raise got %s from %s\n", emission, icon->name);
#endif
  }
}
