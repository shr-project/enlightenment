#include "e.h"
#include "e_mod_main.h"
#ifdef HAVE_WAYLAND
# include "e_mod_comp_wl.h"
# include "e_mod_comp_wl_comp.h"
# include "e_mod_comp_wl_output.h"
# include "e_mod_comp_wl_input.h"
# include "e_mod_comp_wl_shell.h"
# include "e_mod_comp_wl_surface.h"
# include "e_mod_comp_wl_buffer.h"
#endif

/* local function prototypes */
static void _e_mod_comp_wl_surface_buffer_destroy_handle(struct wl_listener *listener, struct wl_resource *resource __UNUSED__, uint32_t timestamp __UNUSED__);
static void _e_mod_comp_wl_surface_raise(Wayland_Surface *ws);
static void _e_mod_comp_wl_surface_damage_rectangle(Wayland_Surface *ws, int32_t x, int32_t y, int32_t width, int32_t height);
static void _e_mod_comp_wl_surface_frame_destroy_callback(struct wl_resource *resource);

Wayland_Surface *
e_mod_comp_wl_surface_create(int32_t x, int32_t y, int32_t w, int32_t h)
{
   Wayland_Surface *ws;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(ws = calloc(1, sizeof(Wayland_Surface)))) return NULL;

   wl_list_init(&ws->link);
   wl_list_init(&ws->buffer_link);

   glGenTextures(1, &ws->texture);
   glBindTexture(GL_TEXTURE_2D, ws->texture);

   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

   ws->surface.resource.client = NULL;

   ws->x = x;
   ws->y = y;
   ws->w = w;
   ws->h = h;
   ws->buffer = NULL;

   ws->win = e_win_new(e_container_current_get(e_manager_current_get()));
   e_win_borderless_set(ws->win, EINA_TRUE);
   e_win_move_resize(ws->win, x, y, w, h);

   pixman_region32_init(&ws->damage);
   pixman_region32_init(&ws->opaque);

   wl_list_init(&ws->frame_callbacks);

   ws->buffer_destroy_listener.func = 
     _e_mod_comp_wl_surface_buffer_destroy_handle;

   /* ws->transform = NULL; */

   return ws;
}

void 
e_mod_comp_wl_surface_destroy(struct wl_client *client __UNUSED__, struct wl_resource *resource)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   wl_resource_destroy(resource, e_mod_comp_wl_time_get());
}

void 
e_mod_comp_wl_surface_attach(struct wl_client *client __UNUSED__, struct wl_resource *resource, struct wl_resource *buffer_resource, int32_t x, int32_t y)
{
   Wayland_Surface *ws;
   struct wl_buffer *buffer;
   struct wl_shell *shell;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   ws = resource->data;
   buffer = buffer_resource->data;
   shell = e_mod_comp_wl_shell_get();

   /* TODO: damage below ?? */

   if (ws->buffer)
     {
        e_mod_comp_wl_buffer_post_release(ws->buffer);
        wl_list_remove(&ws->buffer_destroy_listener.link);
     }

   buffer->busy_count++;
   ws->buffer = buffer;
   wl_list_insert(ws->buffer->resource.destroy_listener_list.prev, 
                  &ws->buffer_destroy_listener.link);

   if (!ws->visual)
     shell->shell.map(&shell->shell, ws, buffer->width, buffer->height);
   else if ((x != 0) || (y != 0) || 
            (ws->w != buffer->width) || (ws->h != buffer->height))
     shell->shell.configure(&shell->shell, ws, ws->x + x, ws->y + y, 
                            buffer->width, buffer->height);

   e_mod_comp_wl_buffer_attach(buffer, &ws->surface);
}

void 
e_mod_comp_wl_surface_damage(struct wl_client *client __UNUSED__, struct wl_resource *resource, int32_t x, int32_t y, int32_t width, int32_t height)
{
   Wayland_Surface *ws;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   ws = resource->data;
   _e_mod_comp_wl_surface_damage_rectangle(ws, x, y, width, height);
}

void 
e_mod_comp_wl_surface_frame(struct wl_client *client, struct wl_resource *resource, uint32_t callback)
{
   Wayland_Surface *ws;
   Wayland_Frame_Callback *cb;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   ws = resource->data;
   if (!(cb = malloc(sizeof(*cb))))
     {
        wl_resource_post_no_memory(resource);
        return;
     }

   cb->resource.object.interface = &wl_callback_interface;
   cb->resource.object.id = callback;
   cb->resource.destroy = _e_mod_comp_wl_surface_frame_destroy_callback;
   cb->resource.client = client;
   cb->resource.data = cb;

   wl_client_add_resource(client, &cb->resource);
   wl_list_insert(ws->frame_callbacks.prev, &cb->link);
}

void 
e_mod_comp_wl_surface_destroy_surface(struct wl_resource *resource)
{
   Wayland_Surface *ws;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   ws = container_of(resource, Wayland_Surface, surface.resource);

   /* TODO: damage below */
   /* TODO: flush damage */

   if (ws->win)
     e_object_del(E_OBJECT(ws->win));

   wl_list_remove(&ws->link);

   if (!ws->saved_texture)
     glDeleteTextures(1, &ws->texture);
   else
     glDeleteTextures(1, &ws->saved_texture);

   if (ws->buffer)
     wl_list_remove(&ws->buffer_destroy_listener.link);

   if (ws->image != EGL_NO_IMAGE_KHR) 
     {
        Wayland_Compositor *comp;

        comp = e_mod_comp_wl_comp_get();
        comp->destroy_image(comp->egl.display, ws->image);
     }

   wl_list_remove(&ws->buffer_link);

   pixman_region32_fini(&ws->damage);
   pixman_region32_fini(&ws->opaque);

   free(ws);
}

void 
e_mod_comp_wl_surface_configure(Wayland_Surface *ws, int32_t x, int32_t y, int32_t width, int32_t height)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ws) return;

   /* TODO: damage below ? */

   ws->x = x;
   ws->y = y;
   ws->w = width;
   ws->h = height;

   if (!wl_list_empty(&ws->frame_callbacks))
     {
        Wayland_Output *output;

        output = e_mod_comp_wl_output_get();
        wl_list_insert_list(output->frame_callbacks.prev, 
                            &ws->frame_callbacks);
        wl_list_init(&ws->frame_callbacks);
     }

   e_mod_comp_wl_surface_damage_surface(ws);

   pixman_region32_fini(&ws->opaque);
   if (ws->visual == WAYLAND_RGB_VISUAL)
     pixman_region32_init_rect(&ws->opaque, ws->x, ws->y, ws->w, ws->h);
   else
     pixman_region32_init(&ws->opaque);

   if (ws->win)
     e_win_move_resize(ws->win, ws->x, ws->y, ws->w, ws->h);
}

void 
e_mod_comp_wl_surface_activate(Wayland_Surface *ws, Wayland_Input *wi, uint32_t timestamp)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (ws->win) e_win_show(ws->win);

   _e_mod_comp_wl_surface_raise(ws);
   wl_input_device_set_keyboard_focus(&wi->input_device, &ws->surface, timestamp);
   wl_data_device_set_keyboard_focus(&wi->input_device);
}

void 
e_mod_comp_wl_surface_damage_surface(Wayland_Surface *ws)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   _e_mod_comp_wl_surface_damage_rectangle(ws, 0, 0, ws->w, ws->h);
}

/* local functions */
static void 
_e_mod_comp_wl_surface_buffer_destroy_handle(struct wl_listener *listener, struct wl_resource *resource __UNUSED__, uint32_t timestamp __UNUSED__)
{
   Wayland_Surface *ws;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   ws = container_of(listener, Wayland_Surface, buffer_destroy_listener);
   ws->buffer = NULL;
}

static void 
_e_mod_comp_wl_surface_raise(Wayland_Surface *ws)
{
   Wayland_Compositor *comp;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   comp = e_mod_comp_wl_comp_get();
   wl_list_remove(&ws->link);
   wl_list_insert(&comp->surfaces, &ws->link);
   /* TODO: repick */
   e_mod_comp_wl_surface_damage_surface(ws);
}

static void 
_e_mod_comp_wl_surface_damage_rectangle(Wayland_Surface *ws, int32_t x, int32_t y, int32_t width, int32_t height)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   pixman_region32_union_rect(&ws->damage, &ws->damage, 
                              ws->x + x, ws->y + y, width, height);
}

static void 
_e_mod_comp_wl_surface_frame_destroy_callback(struct wl_resource *resource)
{
   Wayland_Frame_Callback *cb;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   cb = resource->data;
   wl_list_remove(&cb->link);
   free(cb);
}
