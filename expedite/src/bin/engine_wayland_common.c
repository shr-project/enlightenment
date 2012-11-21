#include <linux/input.h>

#include "main.h"
#include "engine_wayland_common.h"


/* Seat (input) handler */
static void _seat_handle_capabilities(void *data, struct wl_seat *seat, enum wl_seat_capability caps);
static const struct wl_seat_listener engine_wayland_seat_listener =
{
   _seat_handle_capabilities,
};

/* Keyboard handler */
static void _keyboard_handle_keymap(void *data, struct wl_keyboard *keyboard, uint32_t format, int fd, uint32_t size);
static void _keyboard_handle_enter(void *data, struct wl_keyboard *keyboard, uint32_t serial, struct wl_surface *surface, struct wl_array *keys);
static void _keyboard_handle_leave(void *data, struct wl_keyboard *keyboard, uint32_t serial, struct wl_surface *surface);
static void _keyboard_handle_key(void *data, struct wl_keyboard *keyboard, uint32_t serial, uint32_t time, uint32_t key, uint32_t state);
static void _keyboard_handle_modifiers(void *data, struct wl_keyboard *keyboard, uint32_t serial, uint32_t mods_depressed, uint32_t mods_latched, uint32_t mods_locked, uint32_t group);
static const struct wl_keyboard_listener engine_wayland_keyboard_listener =
{
   _keyboard_handle_keymap,
   _keyboard_handle_enter,
   _keyboard_handle_leave,
   _keyboard_handle_key,
   _keyboard_handle_modifiers,
};

/* Shell Surface handler */
static void _shell_surface_handle_ping(void *data, struct wl_shell_surface *shell_surface, uint32_t serial);
static const struct wl_shell_surface_listener _shell_surface_listener =
{
   _shell_surface_handle_ping,
   NULL, /* configure */
   NULL, /* popup_done */
};

/*
 * Public
 */
void
engine_wayland_register_seat(struct wl_registry *registry, unsigned int id)
{
    struct wl_seat *seat = wl_registry_bind(registry, id, &wl_seat_interface, 1);
    wl_seat_add_listener(seat, &engine_wayland_seat_listener, NULL);
}

struct wl_shell_surface *
engine_wayland_create_shell_surface(struct wl_shell *shell, struct wl_surface *surface, const char *title)
{
   struct wl_shell_surface *shell_surface = wl_shell_get_shell_surface(shell, surface);
   wl_shell_surface_set_title(shell_surface, title);
   wl_shell_surface_add_listener(shell_surface, &_shell_surface_listener, NULL);
   wl_shell_surface_set_toplevel(shell_surface);

   return shell_surface;
}

/*
 * Static
 */
static void
_seat_handle_capabilities(void *data __UNUSED__, struct wl_seat *seat, enum wl_seat_capability caps)
{
   static struct wl_keyboard *kbd = NULL;

   if ((caps & WL_SEAT_CAPABILITY_KEYBOARD) && !kbd)
     {
        kbd = wl_seat_get_keyboard(seat);
        wl_keyboard_add_listener(kbd, &engine_wayland_keyboard_listener, NULL);
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

static void
_shell_surface_handle_ping(void *data __UNUSED__, struct wl_shell_surface *shell_surface, uint32_t serial)
{
   wl_shell_surface_pong(shell_surface, serial);
}

