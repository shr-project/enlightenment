#ifndef ENGINE_WAYLAND_COMMON_H
#define ENGINE_WAYLAND_COMMON_H

#include <wayland-client.h>

void engine_wayland_register_seat(struct wl_registry *registry, unsigned int id);

/* Seat (input) handler */
void engine_wayland_seat_handle_capabilities(void *data, struct wl_seat *seat, enum wl_seat_capability caps);

/* Keyboard handler */
void engine_wayland_keyboard_handle_keymap(void *data, struct wl_keyboard *keyboard, uint32_t format, int fd, uint32_t size);
void engine_wayland_keyboard_handle_enter(void *data, struct wl_keyboard *keyboard, uint32_t serial, struct wl_surface *surface, struct wl_array *keys);
void engine_wayland_keyboard_handle_leave(void *data, struct wl_keyboard *keyboard, uint32_t serial, struct wl_surface *surface);
void engine_wayland_keyboard_handle_key(void *data, struct wl_keyboard *keyboard, uint32_t serial, uint32_t time, uint32_t key, uint32_t state);
void engine_wayland_keyboard_handle_modifiers(void *data, struct wl_keyboard *keyboard, uint32_t serial, uint32_t mods_depressed, uint32_t mods_latched, uint32_t mods_locked, uint32_t group);

#endif

