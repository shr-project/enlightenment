#include <string.h>
#include <assert.h>

#include "main.h"
#include "engine_wayland_common.h"

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
   wl.shell_surface = engine_wayland_create_shell_surface(wl.shell, wl.surface, "Expedite Wayland EGL");

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
     wl.compositor = wl_registry_bind(registry, id, &wl_compositor_interface, 1);
   else if (!strcmp(interface, "wl_shell"))
     wl.shell = wl_registry_bind(registry, id, &wl_shell_interface, 1);
   else if (!strcmp(interface, "wl_seat"))
     engine_wayland_register_seat(registry, id);
}
