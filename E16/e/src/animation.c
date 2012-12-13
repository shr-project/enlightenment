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
#include "E.h"
#include "animation.h"
#include "eobj.h"
#include "timers.h"
#include "util.h"
#include <math.h>

#define ENABLE_DEBUG   1
#if ENABLE_DEBUG
#define EDBUG_TYPE_ANIM 180
#define Dprintf(fmt...)  if(EDebug(EDBUG_TYPE_ANIM))Eprintf(fmt)
#define D2printf(fmt...) if(EDebug(EDBUG_TYPE_ANIM)>1)Eprintf(fmt)
#define D3printf(fmt...)
#define EOW(eo) (eo ? EobjGetXwin(eo) : None)
#else
#define Dprintf(fmt...)
#define D2printf(fmt...)
#define D3printf(fmt...)
#endif /* ENABLE_DEBUG */

static int          timing_engine(void);

/*
 * State
 */
static struct {
   Timer              *timer;
   Idler              *idler;
   unsigned int        time_ms;
} Mode_anim =
{
NULL, NULL, 0};

#define FPS 60

static int
_AnimatorsTimer(void *timer_call)
{
   int                 frame_skip;
   int                 dt;

   /* Remember current event time */
   Mode_anim.time_ms = Mode.events.time_ms;

   frame_skip = timing_engine();

   /* time = partial_frames * usecs_per_partial_frame */
   if (frame_skip < 1000000)
      dt = (frame_skip + 1) * (1e3 / FPS);
   else
      dt = 1000000000;		/* Some arbitrary high value */

   if (timer_call)
     {
	TimerSetInterval(Mode_anim.timer, dt);
     }
   else
     {
	TIMER_DEL(Mode_anim.timer);
	TIMER_ADD(Mode_anim.timer, dt, _AnimatorsTimer, (void *)1L);
     }

   D2printf("%s (%s) frame_skip=%d dt=%d\n", __func__,
	    timer_call ? "TIMER" : "IDLER", frame_skip, dt);

   return 1;
}

static void
_AnimatorsIdler(void *data)
{
   /* Don't run idler if we have just run timer */
   if (Mode_anim.time_ms == Mode.events.time_ms)
      return;

   _AnimatorsTimer(data);
}

static void
_AnimatorsInit(void)
{
   TIMER_ADD(Mode_anim.timer, 100, _AnimatorsTimer, (void *)1L);
   Mode_anim.idler = IdlerAdd(_AnimatorsIdler, (void *)0L);
}

/*
 * The animation engine
 */

#define LATER(frame, than)    ((int)(frame - than) > 0)
#define LATER_EQ(frame, than) ((int)(frame - than) >= 0)

struct _animator {
   struct _animator   *next;
   AnimCbFunc         *func;
   AnimDoneFunc       *done;
   animation_category  category;
   int                 duration;
   esound_e            start_sound;
   esound_e            end_sound;
   char                serialize;
   char                cancelled;
   unsigned int        start_frame;
   unsigned int        end_frame;
   unsigned int        next_frame;
   unsigned int        last_tms;
   EObj               *eo;
};

static Animator    *global_animators;

/* This is the frame we THINk we are currently displaying.
 * The next frame to render is this + 1. */
static unsigned int current_frame_num = 1;

/* This is the number of the next frame we need to render for a pending
 * animation */
static unsigned int skip_to_frame_num = 0;

static char         anim_recheck = 0;

Animator           *
AnimatorAdd(EObj * eo, animation_category category, AnimCbFunc * func,
	    int duration, int serialize, size_t data_size, void *data)
{
   Animator           *an, **insert;

   an = (Animator *) calloc(1, sizeof(Animator) + data_size);
   if (!an)
      return NULL;

   Dprintf("%s: %u/%u: %#lx %p C%d\n", __func__,
	   current_frame_num, skip_to_frame_num, EOW(eo), an, category);

   if (!Mode_anim.timer)
      _AnimatorsInit();

   insert = eo ? &eo->animations : &global_animators;
   while (*insert)
      insert = &((*insert)->next);
   *insert = an;

   an->func = func;
   if (duration >= 0)
     {
	an->duration = (duration * FPS) / 1000;	/* ms -> frames */
	if (an->duration == 0)
	   an->duration = 1;	/* At least one frame */
     }
   else
      an->duration = -1;	/* Forever */
   an->category = category;
   an->serialize = serialize;

   an->eo = eo;
   an->start_sound = an->end_sound = SOUND_NONE;

   if (data_size)
      memcpy(an + 1, data, data_size);

   anim_recheck = 1;

   return an;
}

static void
_AnimatorDel(Animator * an)
{
   Dprintf("%s: %u/%u: %#lx %p C%d\n", __func__,
	   current_frame_num, skip_to_frame_num, EOW(an->eo), an, an->category);
   Efree(an);
}

void
AnimatorSetSound(Animator * an, esound_e start_sound, esound_e end_sound)
{
   if (!an)
      return;
   an->start_sound = start_sound;
   an->end_sound = end_sound;
}

void
AnimatorSetDoneFunc(Animator * an, AnimDoneFunc * done)
{
   an->done = done;
}

void
AnimatorsFree(EObj * eo)
{
   Animator           *an, *next;

   for (an = eo->animations; an; an = next)
     {
	next = an->next;
	_AnimatorDel(an);
     }
}

/* Quarter period sinusoidal used in time limited animations */
#define REMAINING(elapsed, duration) \
   (int)(1024 * (1. - cos(((M_PI / 2 * (elapsed)) / (duration)))))

static unsigned int
_AnimatorsRun(Animator ** head, unsigned int frame_num, unsigned int next_frame)
{
   Animator           *an, *next, **pprev;
   int                 res;
   int                 first;
   int                 remaining;
   int                 delta_t;

   for (first = 1, pprev = head, an = *head; an; an = next)
     {
	D3printf("%s: %#lx %p\n", __func__, EOW(an->eo), an);
	next = an->next;

	if (an->cancelled)
	  {
	     res = ANIM_RET_CANCEL_ANIM;
	     goto check_res;
	  }
	if (an->serialize)
	  {
	     /* Start when other non-forever animations have run */
	     if (!first)
		goto do_next;
	     Dprintf("%s: %#lx %p C%d: De-serialize\n", __func__, EOW(an->eo),
		     an, an->category);
	     an->next_frame = frame_num;
	     an->start_frame = an->next_frame;
	     an->end_frame = an->start_frame + an->duration - 1;
	     an->serialize = 0;
	  }
	else if (an->start_frame == an->end_frame)
	  {
	     /* Just added - calculate first/last frame */
	     /* Start "now" or after initial delay (-serialize) */
	     /* NB! New animations start one frame into the future */
	     an->next_frame = current_frame_num + 1 - an->serialize;
	     an->start_frame = an->next_frame;
	     an->end_frame = an->start_frame + an->duration - 1;
	     an->last_tms = Mode_anim.time_ms;
	  }

	/* Don't serialize animations that follow an inf loop with the inf loop */
	if (an->duration > 0)
	   first = 0;

	if (an->category >= 0 && an->duration > 0 &&
	    LATER(an->next_frame, frame_num))
	   goto check_next_frame;

	/*{ start of old _AnimatorRun() */

	if (an->start_sound)
	  {
	     SoundPlay(an->start_sound);
	     an->start_sound = SOUND_NONE;
	  }

	delta_t = Mode_anim.time_ms - an->last_tms;
	an->last_tms = Mode_anim.time_ms;

	if (an->duration > 0)
	  {
	     remaining = 0;
	     if (frame_num < an->end_frame)
		remaining = REMAINING(an->end_frame - frame_num, an->duration);
	  }
	else
	  {
	     remaining = delta_t;
	  }

	D2printf("%s: eo=%p an=%p cat=%d rem=%4d dur=%4d dt=%4d\n", __func__,
		 an->eo, an, an->category, remaining, an->duration, delta_t);
	res = an->func(an->eo, remaining, an + 1);
	Dprintf("%s: res=%4d num=%u end=%u\n", __func__, res, frame_num,
		an->end_frame);

	if (res >= 0)
	  {
	     if (an->duration > 0 && remaining <= 0)
	       {
		  Dprintf("%s: %#lx %p C%d: autocancelling\n", __func__,
			  EOW(an->eo), an, an->category);
		  res = ANIM_RET_CANCEL_ANIM;
	       }
	  }
	else
	  {
	     Dprintf("%s: %#lx %p C%d: self cancelling\n", __func__,
		     EOW(an->eo), an, an->category);
	  }

	/*} end of old _AnimatorRun() */

      check_res:
	if (res >= 0)
	  {
	     /* animator will run again */
	     an->next_frame = frame_num + 1 + res;
	  }
	else
	  {
	     if (an->done)
		an->done(an->eo, an + 1);

	     if (an->end_sound)
		SoundPlay(an->end_sound);

	     _AnimatorDel(an);
	     *pprev = next;
	     continue;		/* Skip pprev update */
	  }

      check_next_frame:
	if (an->category >= 0 && LATER(next_frame, an->next_frame))
	   next_frame = an->next_frame;

      do_next:
	pprev = &an->next;
     }

   return next_frame;
}

static unsigned int
_AnimatorsRunAll(unsigned int frame_num)
{
   EObj              **lst;
   EObj               *const *lst2;
   unsigned int        next_frame;
   int                 num, i;

   lst2 = EobjListStackGet(&num);
   lst = EMALLOC(EObj *, num);
   memcpy(lst, lst2, num * sizeof(EObj *));

   next_frame = frame_num + 0x7fffffff;

   D3printf("%s: %u/%u\n", __func__, current_frame_num, skip_to_frame_num);

   for (i = 0; i < num; i++)
      next_frame = _AnimatorsRun(&lst[i]->animations, frame_num, next_frame);

   Efree(lst);

   next_frame = _AnimatorsRun(&global_animators, frame_num, next_frame);

   return next_frame;
}

int
AnimatorsDelCat(EObj * eo, animation_category category, int complete)
{
   Animator           *an;
   int                 accum = 0;

   Dprintf("%s: cat=%d?\n", __func__, category);

   for (an = (eo) ? eo->animations : global_animators; an; an = an->next)
     {
	if (an->category == category && !an->cancelled)
	  {
	     Dprintf("... %p: complete=%d\n", an, complete);
	     an->cancelled = 1 + complete;
	     accum++;
	  }
     }
   return accum;
}

int
AnimatorsDelCatAll(animation_category category, int complete)
{
   EObj               *const *lst;
   int                 num, i, accum;

   accum = AnimatorsDelCat(NULL, category, complete);
   lst = EobjListStackGet(&num);

   for (i = 0; i < num; i++)
     {
	accum += AnimatorsDelCat(lst[i], category, complete);
     }
   return accum;
}

static unsigned int
_FrameNum(void)
{
   static char         init = 0;
   static unsigned int tp = 0;
   static unsigned int fp = 0;
   unsigned int        t, frame, dx;

   t = GetTimeMs();

   if (!init)
     {
	init = 1;
	tp = t;
     }

   dx = t - tp;
   frame = fp + (dx * FPS) / 1000;

   if (dx > 1000000)
     {
	dx /= 1000;
	tp += dx * 1000;
	fp += dx * FPS;
     }

   return frame;
}

static unsigned int
get_check_frame_count(unsigned int last_frame __UNUSED__,
		      unsigned int skip_to_frame,
		      unsigned int *good_framesp,
		      unsigned int *last_skipped_framep, const char *msg)
{
   unsigned int        frame_num;

   frame_num = _FrameNum();

   if (frame_num > skip_to_frame)
     {
	if (EDebug(1))
	   Eprintf("@%u %s missed %u frames after %u [%u] good frames\n",
		   frame_num, msg, frame_num - skip_to_frame, *good_framesp,
		   skip_to_frame - 1 - *last_skipped_framep);
	*good_framesp = 0;
	*last_skipped_framep = frame_num - 1;
     }

   return frame_num;
}

static int
timing_engine(void)
{
   static unsigned int last_frame_num;
   static unsigned int good_frames;
   static unsigned int last_skipped_frame;
   int                 frameskip;

   current_frame_num = get_check_frame_count(last_frame_num, skip_to_frame_num,
					     &good_frames, &last_skipped_frame,
					     "before render");

   D2printf("%s: cur/last=%u/%u  next=%u  good=%u last-skipped=%u\n",
	    __func__, current_frame_num, last_frame_num, skip_to_frame_num,
	    good_frames, last_skipped_frame);

   if (current_frame_num == last_frame_num && !anim_recheck)
      goto done;

   last_frame_num = current_frame_num;
   anim_recheck = 0;

   skip_to_frame_num = _AnimatorsRunAll(current_frame_num);

 done:
   frameskip = skip_to_frame_num - current_frame_num - 1;

   return frameskip;
}
