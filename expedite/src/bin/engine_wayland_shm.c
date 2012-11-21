#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <sys/mman.h>

#include "main.h"
#include "engine_wayland_common.h"

#include <Evas_Engine_Wayland_Shm.h>
#include <wayland-client.h>

/*
 * Global struct
 */
struct _engine_wayland_shm_display
{
   struct wl_display *display;
   struct wl_compositor *compositor;
   struct wl_surface *surface;
   struct wl_callback *frame_callback;
   struct wl_shell *shell;
   struct wl_shell_surface *shell_surface;
   struct wl_shm *shm;
   struct wl_buffer *buffer;
   void *data;
   int width;
   int height;
};

static struct _engine_wayland_shm_display wl;

/*
 * Function Prototypes
 */
static void _engine_wayland_shm_create_buffer(int width, int height);

/* Registry handler */
static void _registry_handle_global(void *data, struct wl_registry *registry, unsigned int id, const char *interface, unsigned int  version __UNUSED__);
static const struct wl_registry_listener _registry_listener =
{
   _registry_handle_global,
   NULL, /* global_remove */
};

/* Frame handler */
static void _surface_frame_handle_complete(void *data, struct wl_callback *callback, uint32_t time __UNUSED__);
static const struct wl_callback_listener _surface_frame_listener =
{
      _surface_frame_handle_complete,
};


/*
 * API
 */
Eina_Bool
engine_wayland_shm_args(const char *engine __UNUSED__, int width, int height)
{
   struct wl_registry *registry;
   Evas_Engine_Info_Wayland_Shm *einfo;

   evas_output_method_set(evas, evas_render_method_lookup("wayland_shm"));
   einfo = (Evas_Engine_Info_Wayland_Shm *)evas_engine_info_get(evas);
   if (!einfo)
     {
        printf("Evas does not support the Wayland SHM Engine\n");
        return EINA_FALSE;
     }

   wl.display = wl_display_connect(NULL);
   registry = wl_display_get_registry(wl.display);
   wl_registry_add_listener(registry, &_registry_listener, NULL);
   wl_display_roundtrip(wl.display);

   assert(wl.compositor != NULL);
   assert(wl.shell != NULL);
   assert(wl.shm != NULL);

   wl.surface = wl_compositor_create_surface(wl.compositor);
   wl.shell_surface = engine_wayland_create_shell_surface(wl.shell, wl.surface, "Expedite Wayland SHM");

   _engine_wayland_shm_create_buffer(width, height);

   assert(wl.buffer != NULL);
   assert(wl.data != NULL);

   wl_surface_attach(wl.surface, wl.buffer, 0, 0);

   einfo->info.dest = wl.data;
   if (!evas_engine_info_set(evas, (Evas_Engine_Info *) einfo))
     {
        printf("Evas can not setup the informations of the Wayland SHM Engine\n");
        return EINA_FALSE;
     }

   wl.width = width;
   wl.height = height;

   _surface_frame_handle_complete(NULL, NULL, 0);

   return EINA_TRUE;
}

void
engine_wayland_shm_loop(void)
{
   assert(wl_display_dispatch(wl.display) != -1);
}


void
engine_wayland_shm_shutdown(void)
{
   if (wl.frame_callback)
      wl_callback_destroy(wl.frame_callback);

   wl_buffer_destroy(wl.buffer);
   wl_shell_surface_destroy(wl.shell_surface);
   wl_surface_destroy(wl.surface);
   wl_shm_destroy(wl.shm);
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
     wl.compositor = wl_registry_bind(registry, id, &wl_compositor_interface, 1);
   else if (!strcmp(interface, "wl_shell"))
     wl.shell = wl_registry_bind(registry, id, &wl_shell_interface, 1);
   else if (!strcmp(interface, "wl_shm"))
     wl.shm = wl_registry_bind(registry, id, &wl_shm_interface, 1);
   else if (!strcmp(interface, "wl_seat"))
     engine_wayland_register_seat(registry, id);
}

static void
_engine_wayland_shm_create_buffer(int width, int height)
{
   struct wl_shm_pool *pool;
   int fd, size, stride;
   char tmp[PATH_MAX];

   stride = width * 4;
   size = stride * height;

   strcpy(tmp, "/tmp/expedite-wayland_shm-XXXXXX");
   if ((fd = mkstemp(tmp)) < 0)
     {
        fprintf(stderr, "Could not create temporary file.\n");
        return;
     }

   if (ftruncate(fd, size) < 0)
     {
        fprintf(stderr, "Could not truncate temporary file.\n");
        goto end;
     }

   wl.data = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
   if (wl.data == MAP_FAILED)
     {
        wl.data = NULL;
        fprintf(stderr, "mmap failed\n");
        goto end;
     }

   pool = wl_shm_create_pool(wl.shm, fd, size);
   wl.buffer = wl_shm_pool_create_buffer(pool, 0, width, height, stride, WL_SHM_FORMAT_XRGB8888);
   wl_shm_pool_destroy(pool);

 end:
   close(fd);
}

static void
_surface_frame_handle_complete(void *data __UNUSED__, struct wl_callback *callback __UNUSED__, uint32_t time __UNUSED__)
{
   wl_surface_damage(wl.surface, 0, 0, wl.width, wl.height);

   if (wl.frame_callback)
      wl_callback_destroy(wl.frame_callback);

   wl.frame_callback = wl_surface_frame(wl.surface);
   wl_callback_add_listener(wl.frame_callback, &_surface_frame_listener, NULL);

   wl_surface_commit(wl.surface);
}

