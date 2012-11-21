#include "main.h"

#include <string.h>
#include <assert.h>

#include <linux/input.h>

#include <Evas_Engine_Wayland_Egl.h>
#include <wayland-client.h>
#include <wayland-egl.h>

/* Global struct */
struct _engine_wayland_egl_display
{
   struct wl_display *display;
   struct wl_compositor *compositor;
   struct wl_surface *surface;
   struct wl_shell *shell;
   struct wl_shell_surface *shell_surface;
};

static struct _engine_wayland_egl_display wl;

/*
 * Function prototypes
 */
/* Registry handler */
static void _registry_handle_global(void *data, struct wl_registry *registry, unsigned int id, const char *interface, unsigned int  version __UNUSED__);
static const struct wl_registry_listener _registry_listener =
{
   _registry_handle_global,
   NULL, /* global_remove */
};

/* Shell Surface handler */
static void _shell_surface_handle_ping(void *data, struct wl_shell_surface *shell_surface, uint32_t serial);
static const struct wl_shell_surface_listener _shell_surface_listener =
{
   _shell_surface_handle_ping,
   NULL, /* configure */
   NULL, /* popup_done */
};

/* Seat (input) handler */
static void _seat_handle_capabilities(void *data, struct wl_seat *seat, enum wl_seat_capability caps);
static const struct wl_seat_listener _seat_listener =
{
   _seat_handle_capabilities,
};

/* Keyboard handler */
static void _keyboard_handle_keymap(void *data, struct wl_keyboard *keyboard, uint32_t format, int fd, uint32_t size);
static void _keyboard_handle_enter(void *data, struct wl_keyboard *keyboard, uint32_t serial, struct wl_surface *surface, struct wl_array *keys);
static void _keyboard_handle_leave(void *data, struct wl_keyboard *keyboard, uint32_t serial, struct wl_surface *surface);
static void _keyboard_handle_key(void *data, struct wl_keyboard *keyboard, uint32_t serial, uint32_t time, uint32_t key, uint32_t state);
static void _keyboard_handle_modifiers(void *data, struct wl_keyboard *keyboard, uint32_t serial, uint32_t mods_depressed, uint32_t mods_latched, uint32_t mods_locked, uint32_t group);
static const struct wl_keyboard_listener _keyboard_listener =
{
   _keyboard_handle_keymap, /* keymap */
   _keyboard_handle_enter, /* enter */
   _keyboard_handle_leave, /* leave */
   _keyboard_handle_key,
   _keyboard_handle_modifiers, /* modifiers */
};

/*
 * API
 */
Eina_Bool
engine_wayland_egl_args(const char *engine __UNUSED__, int width __UNUSED__, int height __UNUSED__)
{
   struct wl_registry *registry;
   Evas_Engine_Info_Wayland_Egl *einfo;

   evas_output_method_set(evas, evas_render_method_lookup("wayland_egl"));
   einfo = (Evas_Engine_Info_Wayland_Egl *)evas_engine_info_get(evas);
   if (!einfo)
     {
        printf("Evas does not support the Wayland EGL Engine\n");
        return EINA_FALSE;
     }

   wl.display = wl_display_connect(NULL);
   registry = wl_display_get_registry(wl.display);
   wl_registry_add_listener(registry, &_registry_listener, NULL);
   wl_display_roundtrip(wl.display);

   assert(wl.compositor != NULL);
   assert(wl.shell != NULL);

   wl.surface = wl_compositor_create_surface(wl.compositor);
   wl.shell_surface = wl_shell_get_shell_surface(wl.shell, wl.surface);
   wl_shell_surface_set_title(wl.shell_surface, "Expedite Wayland EGL");
   wl_shell_surface_add_listener(wl.shell_surface, &_shell_surface_listener, NULL);
   wl_shell_surface_set_toplevel(wl.shell_surface);

   einfo->info.display = wl.display;
   einfo->info.surface = wl.surface;
   if (!evas_engine_info_set(evas, (Evas_Engine_Info *) einfo))
     {
        printf("Evas can not setup the informations of the Wayland EGL Engine\n");
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

void
engine_wayland_egl_loop(void)
{
   assert(wl_display_dispatch(wl.display) != -1);
}

void
engine_wayland_egl_shutdown(void)
{
   wl_shell_surface_destroy(wl.shell_surface);
   wl_surface_destroy(wl.surface);
   wl_shell_destroy(wl.shell);
   wl_compositor_destroy(wl.compositor);
   wl_display_flush(wl.display);
   wl_display_disconnect(wl.display);
}

/*
 * Function implementation
 */
static void
_registry_handle_global(void *data __UNUSED__, struct wl_registry *registry, unsigned int id, const char *interface, unsigned int  version __UNUSED__)
{
   if (!strcmp(interface, "wl_compositor"))
     {
        wl.compositor = wl_registry_bind(registry, id, &wl_compositor_interface, 1);
     }
   else if (!strcmp(interface, "wl_shell"))
     {
        wl.shell = wl_registry_bind(registry, id, &wl_shell_interface, 1);
     }
   else if (!strcmp(interface, "wl_seat"))
     {
        struct wl_seat *seat;
        seat = wl_registry_bind(registry, id, &wl_seat_interface, 1);
        wl_seat_add_listener(seat, &_seat_listener, NULL);
     }
}

static void
_shell_surface_handle_ping(void *data __UNUSED__, struct wl_shell_surface *shell_surface, uint32_t serial)
{
   wl_shell_surface_pong(shell_surface, serial);
}

static void
_seat_handle_capabilities(void *data __UNUSED__, struct wl_seat *seat, enum wl_seat_capability caps)
{
   static struct wl_keyboard *kbd = NULL;

   if ((caps & WL_SEAT_CAPABILITY_KEYBOARD) && !kbd)
     {
        kbd = wl_seat_get_keyboard(seat);
        wl_keyboard_add_listener(kbd, &_keyboard_listener, NULL);
     }
   else if (!(caps & WL_SEAT_CAPABILITY_KEYBOARD) && kbd)
     {
        wl_keyboard_destroy(kbd);
        kbd = NULL;
     }
}

static void
_keyboard_handle_keymap(void *data __UNUSED__, struct wl_keyboard *keyboard __UNUSED__, uint32_t format __UNUSED__, int fd __UNUSED__, uint32_t size __UNUSED__)
{
}

static void
_keyboard_handle_enter(void *data __UNUSED__, struct wl_keyboard *keyboard __UNUSED__, uint32_t serial __UNUSED__, struct wl_surface *surface __UNUSED__, struct wl_array *keys __UNUSED__)
{
}

static void
_keyboard_handle_leave(void *data __UNUSED__, struct wl_keyboard *keyboard __UNUSED__, uint32_t serial __UNUSED__, struct wl_surface *surface __UNUSED__)
{
}

static void
_keyboard_handle_key(void *data __UNUSED__, struct wl_keyboard *keyboard __UNUSED__, uint32_t serial __UNUSED__, uint32_t time __UNUSED__, uint32_t key, uint32_t state)
{
   const char *key_str;

   switch (key)
     {
      case KEY_LEFT:
         key_str = "Left";
         break;

      case KEY_RIGHT:
         key_str = "Right";
         break;

      case KEY_ENTER:
      case KEY_KPENTER:
         key_str = "Return";
         break;

      case KEY_ESC:
         key_str = "Escape";
         break;

      default:
         key_str = NULL;
         break;
     }

   if (key_str)
     {
        switch (state)
          {
           case WL_KEYBOARD_KEY_STATE_RELEASED:
              evas_event_feed_key_up(evas, key_str, key_str, NULL, NULL, 0, NULL);
              break;

           case WL_KEYBOARD_KEY_STATE_PRESSED:
              evas_event_feed_key_down(evas, key_str, key_str, NULL, NULL, 0, NULL);
              break;

           default:
              break;
          }
     }
}

static void
_keyboard_handle_modifiers(void *data __UNUSED__, struct wl_keyboard *keyboard __UNUSED__, uint32_t serial __UNUSED__, uint32_t mods_depressed __UNUSED__, uint32_t mods_latched __UNUSED__, uint32_t mods_locked __UNUSED__, uint32_t group __UNUSED__)
{
}
