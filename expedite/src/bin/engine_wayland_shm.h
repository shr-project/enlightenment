#ifndef ENGINE_WAYLAND_SHM_H
#define ENGINE_WAYLAND_SHM_H

Eina_Bool engine_wayland_shm_args(const char *engine, int width, int height);
void engine_wayland_shm_loop(void);
void engine_wayland_shm_shutdown(void);

#endif
