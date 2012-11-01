#ifndef ENGINE_WAYLAND_EGL_H
#define ENGINE_WAYLAND_EGL_H

Eina_Bool engine_wayland_egl_args(const char *engine, int width, int height);
void engine_wayland_egl_loop(void);
void engine_wayland_egl_shutdown(void);

#endif
