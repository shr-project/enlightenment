#include <Efx.h>
#include <Ecore.h>
#include <Ecore_Evas.h>

#ifndef __UNUSED__
# define __UNUSED__ __attribute__((unused))
#endif

static Eina_Bool _move1(Evas_Object *r);

static Eina_Bool
_move4(Evas_Object *r)
{
   efx_move(r, EFX_EFFECT_SPEED_DECELERATE, &(Evas_Point){ 25, 25 }, 3.0);
   ecore_timer_add(3.0, (Ecore_Task_Cb)_move1, r);
   return EINA_FALSE;
}

static Eina_Bool
_move3(Evas_Object *r)
{
   efx_move(r, EFX_EFFECT_SPEED_ACCELERATE, &(Evas_Point){ 25, 350 }, 3.0);
   ecore_timer_add(3.0, (Ecore_Task_Cb)_move4, r);
   return EINA_FALSE;
}

static Eina_Bool
_move2(Evas_Object *r)
{
   efx_move(r, EFX_EFFECT_SPEED_DECELERATE, &(Evas_Point){ 350, 350 }, 3.0);
   ecore_timer_add(3.0, (Ecore_Task_Cb)_move3, r);
   return EINA_FALSE;
}

static Eina_Bool
_move1(Evas_Object *r)
{
   efx_move(r, EFX_EFFECT_SPEED_ACCELERATE, &(Evas_Point){ 350, 25 }, 3.0);
   ecore_timer_add(3.0, (Ecore_Task_Cb)_move2, r);
   return EINA_FALSE;
}

static Eina_Bool
_start(Evas_Object *r)
{
   _move1(r);
   return EINA_FALSE;
}

static void
_end(Ecore_Evas *ee __UNUSED__)
{
   ecore_main_loop_quit();
}

int
main(void)
{
   Ecore_Evas *ee;
   Evas *e;
   Evas_Object *r;

   efx_init();
   ecore_evas_init();
   eina_log_domain_level_set("efx", EINA_LOG_LEVEL_DBG);
   ee = ecore_evas_software_x11_new(NULL, 0, 0, 0, 450, 450);
   ecore_evas_callback_delete_request_set(ee, _end);
   ecore_evas_show(ee);
   e = ecore_evas_get(ee);
   r = evas_object_rectangle_add(e);
   evas_object_resize(r, 450, 450);
   evas_object_show(r);

   r = evas_object_rectangle_add(e);
   evas_object_color_set(r, 255, 0, 0, 255);
   evas_object_resize(r, 75, 75);
   evas_object_move(r, 25, 25);
   evas_object_show(r);

   ecore_timer_add(1.0, (Ecore_Task_Cb)_start, r);
   ecore_main_loop_begin();
   return 0;
}
