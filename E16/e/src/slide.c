/*
 * Copyright (C) 2012 Kim Woelders
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies of the Software, its documentation and marketing & publicity
 * materials, and acknowledgment shall be given in the documentation, materials
 * and software packages that this Software was used.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "E.h"
#include "animation.h"
#include "eobj.h"
#include "ewins.h"
#include "focus.h"
#include "slide.h"
#include "xwin.h"

/*
 * EObj sliding functions
 */

typedef struct {
   int                 fx, fy, fw, fh;
   int                 tx, ty, tw, th;
} eobj_slide_params;

static int
_EobjSlideSizeTo(EObj * eo, int remaining, void *state)
{
   eobj_slide_params  *params = (eobj_slide_params *) state;
   int                 k = 1024 - remaining, x, y, w, h;

   x = ((params->fx * (1024 - k)) + (params->tx * k)) >> 10;
   y = ((params->fy * (1024 - k)) + (params->ty * k)) >> 10;
   w = ((params->fw * (1024 - k)) + (params->tw * k)) >> 10;
   h = ((params->fh * (1024 - k)) + (params->th * k)) >> 10;
   EobjMoveResize(eo, x, y, w, h);

   return 0;
}

void
EobjSlideSizeTo(EObj * eo, int fx, int fy, int tx, int ty, int fw, int fh,
		int tw, int th, int speed)
{
   eobj_slide_params   params;
   int                 duration;

   params.fx = fx;
   params.fy = fy;
   params.fw = fw;
   params.fh = fh;
   params.tx = tx;
   params.ty = ty;
   params.tw = tw;
   params.th = th;

   if (speed <= 10)
      speed = 10;
   duration = 1000000 / speed;

   AnimatorAdd(eo, ANIM_SLIDE, _EobjSlideSizeTo, duration, 1,
	       sizeof(params), &params);
}

/*
 * EWin sliding functions
 */

typedef struct {
   int                 fx, fy, fw, fh;
   int                 tx, ty, tw, th;
   int                 mode;
   char                mouse_warp;
   char                give_focus;
   int                 mouse_x;
   int                 mouse_y;
} ewin_slide_params;

static int
_EwinSlideSizeTo(EObj * eo, int remaining, void *state)
{
   ewin_slide_params  *params = (ewin_slide_params *) state;
   EWin               *ewin = (EWin *) eo;
   int                 k = 1024 - remaining, x, y, w, h;

   x = ((params->fx * (1024 - k)) + (params->tx * k)) >> 10;
   y = ((params->fy * (1024 - k)) + (params->ty * k)) >> 10;
   w = ((params->fw * (1024 - k)) + (params->tw * k)) >> 10;
   h = ((params->fh * (1024 - k)) + (params->th * k)) >> 10;

   EwinMoveResize(ewin, x, y, w, h, MRF_KEEP_MAXIMIZED);
   EwinShapeSet(ewin);

   if (params->mouse_warp)
     {
	EwinWarpTo(ewin, 1);
	EWarpPointer(EoGetWin(ewin), params->mouse_x, params->mouse_y);
     }

   if (!remaining)
     {
	ewin->state.sliding = 0;
	if (params->give_focus)
	  {
	     FocusToEWin(ewin, FOCUS_SET);
	  }
     }

   return 0;
}

Animator           *
EwinSlideSizeTo(EWin * ewin, int tx, int ty, int tw, int th,
		int speed, int mode, int flags)
{
   Animator           *an;
   ewin_slide_params   params;
   int                 duration;
   esound_e            start_sound = SOUND_NONE;
   esound_e            end_sound = SOUND_NONE;

   ewin->state.sliding = 1;

   params.fx = EoGetX(ewin);
   params.fy = EoGetY(ewin);
   params.fw = ewin->client.w;
   params.fh = ewin->client.h;
   params.tx = tx;
   params.ty = ty;
   params.tw = tw;
   params.th = th;
   params.mode = mode;
   params.give_focus = (flags & SLIDE_FOCUS) != 0;
   params.mouse_warp = ((flags & SLIDE_WARP) != 0) &&
      ((params.fx != params.tx) || (params.fy != params.ty)) &&
      (ewin == GetEwinPointerInClient());
   EQueryPointer(EoGetWin(ewin), &params.mouse_x, &params.mouse_y, NULL, NULL);

   if (params.mouse_x > tw)
      params.mouse_x = tw / 2;
   if (params.mouse_y > th)
      params.mouse_y = th / 2;

   if (flags & SLIDE_SOUND)
     {
	start_sound = SOUND_WINDOW_SLIDE;
	end_sound = SOUND_WINDOW_SLIDE_END;
     }

   if (speed <= 10)
      speed = 10;
   duration = 1000000 / speed;

   an = AnimatorAdd((EObj *) ewin, ANIM_SLIDE, _EwinSlideSizeTo, duration, 0,
		    sizeof(params), &params);
   AnimatorSetSound(an, start_sound, end_sound);

   return an;
}

Animator           *
EwinSlideTo(EWin * ewin, int fx __UNUSED__, int fy __UNUSED__, int tx, int ty,
	    int speed, int mode, int flags)
{
// EwinMove(ewin, fx, fy, 0);   // FIXME - WHY?
   return EwinSlideSizeTo(ewin, tx, ty, ewin->client.w, ewin->client.h,
			  speed, mode, flags);
}

Animator           *
EwinsSlideTo(EWin ** ewin, int *fx, int *fy, int *tx, int *ty, int num_wins,
	     int speed, int mode, int flags)
{
   Animator           *an = NULL;
   int                 i;

   for (i = 0; i < num_wins; i++)
     {
	an =
	   EwinSlideTo(ewin[i], fx[i], fy[i], tx[i], ty[i], speed, mode, flags);
	flags |= SLIDE_SOUND;
     }

   return an;
}
