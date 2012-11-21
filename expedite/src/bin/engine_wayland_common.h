#ifndef ENGINE_WAYLAND_COMMON_H
#define ENGINE_WAYLAND_COMMON_H

#include <wayland-client.h>

void engine_wayland_register_seat(struct wl_registry *registry, unsigned int id);
struct wl_shell_surface *engine_wayland_create_shell_surface(struct wl_shell *shell, struct wl_surface *surface, const char *title);

#endif

