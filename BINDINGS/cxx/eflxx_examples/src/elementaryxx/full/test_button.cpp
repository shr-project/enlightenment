#include "test.h"

void test_button (void *data, Evas_Object *obj, void *event_info)
{
  Icon *ic = NULL;
  Button *bt = NULL;

  Window *win = Window::factory ("buttons", ELM_WIN_BASIC);
  win->setTitle ("Buttons");
  win->setAutoDel (true);

  Background *bg = Background::factory (*win);
  win->addResizeObject (*bg);
  bg->setSizeHintWeight (EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
  bg->show ();

  Box *bx = Box::factory (*win);
  win->addResizeObject (*bx);
  bx->setSizeHintWeight (EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
  bx->show ();

  ic = Icon::factory (*win);
  ic->setFile (searchPixmapFile ("elementaryxx/logo_small.png"));
  ic->setSizeHintAspect (EVAS_ASPECT_CONTROL_VERTICAL, Size (1, 1));
  bt = Button::factory (*win);
  bt->setText ("Icon sized to button");
  bt->setContent (*ic);
  bx->packEnd (*bt);
  bt->show ();
  ic->show ();

  ic = Icon::factory (*win);
  ic->setFile (searchPixmapFile ("elementaryxx/logo_small.png"));
  ic->setSizeHintAspect (EVAS_ASPECT_CONTROL_VERTICAL, Size (1, 1));
  bt = Button::factory (*win);
  bt->setText ("Disabled Button");
  bt->setContent (*ic);
  bt->setDisabled (true);
  bx->packEnd (*bt);
  bt->show ();
  ic->show ();

  ic = Icon::factory (*win);
  ic->setFile (searchPixmapFile ("elementaryxx/logo_small.png"));
  ic->setNoScale (true);
  bt = Button::factory (*win);
  bt->setText ("Icon no scale");
  bt->setContent (*ic);
  bx->packEnd (*bt);
  bt->show ();
  ic->show ();

  bt = Button::factory (*win);
  bt->setText ("Label Only");
  bx->packEnd (*bt);
  bt->show ();

  ic = Icon::factory (*win);
  ic->setFile (searchPixmapFile ("elementaryxx/logo_small.png"));
  ic->setNoScale (true);
  bt = Button::factory (*win);
  bt->setText ("Icon no scale");
  bt->setContent (*ic);
  bx->packEnd (*bt);
  bt->show ();
  ic->show ();

  win->show ();
}