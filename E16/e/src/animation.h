/*
 * Copyright (C) 2012 Daniel Manjarres
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
#ifndef _ANIMATION_H_
#define _ANIMATION_H_

#include "eobj.h"

/*
 * Animator function prototype.
 *
 * eo  : A pointer to the pertinent EObj.
 * run : A parameter indicating animation progress.
 *       If the animation time is limited (duration > 0), run will go
 *       from ~1023 to 0.
 *       The first call will most likely not have run=1023, but (unless
 *       the animator is killed) the last call will have run=0.
 *       If the animation is not time limited, the first call will
 *       have run=0 (TBD), and the subsequent calls will have run equal
 *       to the time since last call in ms.
 * data: A pointer to extra animator data, as specified when creating the
 *       animator.
 *
 * Because a EObj may be destroyed before then animation is complete, it
 * must be safe to simply free the Animator struct and get on with life,
 * so animations should not require destructor-like functions to run after
 * they are complete.
 */
typedef int         (AnimCbFunc) (EObj * eo, int run, void *data);
typedef void        (AnimDoneFunc) (EObj * eo, void *data);

/*
 * If AnimCbFunc's (retval > 0) it will not be called for retval frames.
 * If AnimCbFunc's (retval == -1) the animation is cancelled.
 */
#define ANIM_RET_CANCEL_ANIM	-1

typedef enum {
   /* lazy animations have negative names, and
    * do not trigger frames to be drawn, but do draw things
    * when other things trigger frames */
   ANIM_LAZY_MAGWIN = -8,

   ANIM_NOT_USED = 0,

   ANIM_FADE,
   ANIM_SLIDE,
   ANIM_SHADE,

   ANIM_STARTUP = 20,
   ANIM_GLWIN,

   /* not a window animation, but a desktop animation */
   ANIM_FX_SPINNER = 40,
   ANIM_FX_RAINDROPS,
   ANIM_FX_WAVES,
   ANIM_FX_RIPPLES,
} animation_category;

Animator           *AnimatorAdd(EObj * eo, animation_category category,
				AnimCbFunc * func, int duration, int serialize,
				size_t data_size, void *data);
void                AnimatorSetSound(Animator * an,
				     esound_e start_sound, esound_e end_sound);
void                AnimatorSetDoneFunc(Animator * an, AnimDoneFunc * done);

int                 AnimatorsDelCat(EObj * eo, animation_category category,
				    int complete);
int                 AnimatorsDelCatAll(animation_category category,
				       int complete);

void                AnimatorsFree(EObj * eo);

#endif /* _ANIMATION_H_ */
