/*****************************************************************************/
/* Enlightenment - The Window Manager that dares to do what others don't     */
/*****************************************************************************/
/* Copyright (C) 1997 - 1999 Carsten Haitzler (The Rasterman)                */
/*                                                                           */
/* This program and utilites is free software; you can redistribute it       */
/* and/or modify it under the terms of the License shown in COPYING          */
/*                                                                           */
/* This software is distributed in the hope that it will be useful,          */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of            */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                      */
/*****************************************************************************/

#include <X11/Xlib.h>
#include <X11/Xresource.h>
#include <X11/Xutil.h>
#include <Imlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <pwd.h>
#include <sys/types.h>

/****************************************************************************/
/* Data structures & primitives                                             */
/****************************************************************************/
typedef struct _etimer ETimer;
typedef void * Epplet_gadget;
typedef struct _rgb_buf
{
   ImlibImage *im;
} * RGB_buf;

/****************************************************************************/
/* Initialization call                                                      */
/****************************************************************************/
/* this sets everything up - connects to X, finds E, creates a window... */
/* you pass you name, version and info (eg "Clock_App", "0.2", */
/* "Enlightenment Mini-Clock Application"), then widht and height in w, h. */
/* width and height are multiples of 16 pixels so (2, 2) will make a 32x32 */
/* sized window. You also ned to pass your argc and argv paramaters your */
/* main() function gets. vertical is a flag as to if the app is vertical. */
/* apps more horizontal than vertical shodul set this to 0. */
void   Epplet_Init(char *name, char *version, char *info, int w, int h,
		   int argc, char **argv, char vertical);
/* actualy display the app */
void   Epplet_show(void);
/* ask E to remember stuff about it - you don't need to do this at startup */
/* or whenver the Epplet moves etc.- this is done for you, but if you need */
/* to for some special reason - call it */
void   Epplet_remember(void);
/* if you dont want E to rememebr anything abotu you or start you up anymore */
/* call this - this is a good call to make if you want to exit and never */
/* have yourself started up again by E */
void   Epplet_unremember(void);
/* return the window id of the main epplet window */
Window Epplet_get_main_window(void);
/* return the X display connection used */
Display *Epplet_get_display(void);

/****************************************************************************/
/* IPC calls                                                                */
/****************************************************************************/
/* send the string "s" to Enlightenment */
void   Epplet_send_ipc(char *s);
/* sit and wait for an IPc message - nothing happens whilst waiting no */
/* timers run, no events or anything else is handled. */
char  *Epplet_wait_for_ipc(void);

/* take the imageclass called iclass in state state ("normal", "hilited", */
/* "clicked") and set it as the backgorund pixmap to window ww and have */
/* its shape mask be the shape of the window ww */
void   Epplet_imageclass_apply(char *iclass, char *state, Window ww);
/* paste the imageclass iclass in state state in window ww at (x,y) at a */
/* size of (w x h) */
void   Epplet_imageclass_paste(char *iclass, char *state, Window ww, 
			       int x, int y, int w, int h);
/* return pixmaps of imageclass iclass in the state state and place the */
/* pixmap and mask ID's in the Pixmaps Id's poitned to by p and m, and have */
/* the pximaps be of size (w x h) */
void   Epplet_imageclass_get_pixmaps(char *iclass, char *state, Pixmap *p, 
				     Pixmap *m, int w, int h);
/* draw the text class tclass in state state on window ww at x, y with the */
/* text "txt" */
void   Epplet_textclass_draw(char *tclass, char *state, Window ww, int x, 
			     int y, char *txt);
/* get the size text for textclass tclass will be using the text "txt" and */
/* return widht and height to the int's pointed to by e and h */
void   Epplet_textclass_get_size(char *tclass, int *w, int *h, char *txt);
/* the epplet main loop - once you've set up and showed your epplet window */
/* call this */
void   Epplet_Loop(void);
/* call the function func with data as its data param whenever an expose */
/* happens and needs to be handled */
void   Epplet_register_expose_handler(void (*func)
				      (void *data, Window win, int x, int y, int w, int h),
				      void *data);
/* call func whenever the epplet is moved */
void   Epplet_register_move_resize_handler(void (*func)
					   (void *data, Window win, int x, int y, int w, int h),
					   void *data);
/* call func whenever a button is pressed */
void   Epplet_register_button_press_handler(void (*func)
					    (void *data, Window win, int x, int y, int b),
					    void *data);
/* call func whenever a button is released */
void   Epplet_register_button_release_handler(void (*func)
					      (void *data, Window win, int x, int y, int b),
					      void *data);
/* call func whenever a key is pressed (pass a string version of the key */
/* pressed to the regsitsered function) */
void   Epplet_register_key_press_handler(void (*func)
					 (void *data, Window win, char *key),
					 void *data);
/* call func whenever a key is released (pass a string version of the key */
/* pressed to the regsitsered function) */
void   Epplet_register_key_release_handler(void (*func)
					   (void *data, Window win, char *key),
					   void *data);
/* call func whenever a the mouse is moved in a window */
void   Epplet_register_mouse_motion_handler(void (*func)
					    (void *data, Window win, int x, int y),
					    void *data);
/* call func whenever a the mouse enters a window */
void   Epplet_register_mouse_enter_handler(void (*func)
					   (void *data, Window win),
					   void *data);
/* call func whenever a the mouse leaves a window */
void   Epplet_register_mouse_leave_handler(void (*func)
					   (void *data, Window win),
					   void *data);
/* call func whenever focus is active on your epplet window */
void   Epplet_register_focus_in_handler(void (*func)
					(void *data, Window win),
					void *data);
/* call func whenever leaves your epplet window */
void   Epplet_register_focus_out_handler(void (*func)
					 (void *data, Window win),
					 void *data);
/* call func and pass a pointer to n XEvent on every event that happens */
void   Epplet_register_event_handler(void (*func)
				     (void *data, XEvent *ev),
				     void *data);
/* call func and pass a string (that you dont have to free) with the IPC */
/* message whenever e sends you an IPC message */
void   Epplet_register_comms_handler(void (*func)
				     (void *data, char *s),
				     void *data);

/****************************************************************************/
/* Timer timeout functions - this lets you appear to have threads when you  */
/* dont have any at all                                                     */
/****************************************************************************/
/* run function func and pass data data to it in in seconds (in is double */
/* so you can for exmaple use 0.5 to have that function called in 0.5 */
/* seconds from now ). You also attach the name to the timeout of name */
void   Epplet_timer(void (*func) (void *data), void *data, double in, 
		    char *name);
/* delete any timeout of name name in the queue. you should use unique */
/* names for different timeouts in the queue */
void   Epplet_remove_timer(char *name);
/* get the current time as a double (time is in seconds since Jan 1, 1970 */
double Epplet_get_time(void);

/****************************************************************************/
/* widgets available from the epplet api that use images from E to define   */
/* their look. if you change themes in E all the widgets will change too    */
/****************************************************************************/
/* create a button with either label label or image file image at (x,y) with */
/* with a size of (w x h), OR make a standard button of name std whihc is */
/* (12x12) pixels in size. Valid standard butotns are: "ARROW_UP", */
/* "ARROW_DOWN", "ARROW_LEFT", "ARROW_RIGHT", "PLAY", "STOP", "PAUSE", */
/* "PREVIOUS", "NEXT", "EJECT", "CLOSE", "FAST_FORWARD", "REWIND", "REPEAT", */
/* "SKIP", "HELP", "CONFIGURE" */
/* parent is the parent window - normally you only need to use 0 for this */
/* unless you want a special parent (only buttons can be speically parented */
/* and the function func si called whne the button is clicked and data is */
/* passed to that function */
Epplet_gadget   Epplet_create_button(char *label, char *image, int x, int y,
				     int w, int h, char *std, Window parent,
				     Epplet_gadget pop_parent,
				     void (*func) (void *data), void *data);
/* create drawing area at (x,y) of size (w x h) */
Epplet_gadget   Epplet_create_drawingarea(int x, int y, int w, int h);
/* create horizontal slider at x, y of length len. the minimum length is 9 */
/* pixels, and the width is always 8 pixels. min is the minimum value and */
/* max is the maximum value. max should always > min. the slider can move */
/* by step units as a minimum step, and moves by jump whenever you click */
/* either side of the slider. whenever the slider changed func is called */
Epplet_gadget   Epplet_create_hslider(int x, int y, int len, int min, int max,
				      int step, int jump, int *val,
				      void (*func) (void *data), void *data);
/* same as horizontal slider except vertical */
Epplet_gadget   Epplet_create_vslider(int x, int y, int len, int min, int max,
				      int step, int jump, int *val,
				      void (*func) (void *data), void *data);
/* create a button (like normal buttons) except it toggles the value */
/* pointed to by val between 1 and 0. func is called whenever it changes */
Epplet_gadget   Epplet_create_togglebutton(char *label, char *pixmap, int x, 
					   int y, int w, int h, int *val, 
					   void (*func) (void *data),
					   void *data);
/* creates a button just like normal button except it pops up the popup */
/* when clicked */
Epplet_gadget   Epplet_create_popupbutton(char *label, char *image, int x, 
					  int y, int w, int h,
					  Epplet_gadget popup);
/* creates an empty popup */
Epplet_gadget   Epplet_create_popup(void);
/* adds an image file pixmaps or label to the popup gadget and calls */
/* func when it is selected */
void            Epplet_add_popup_entry(Epplet_gadget gadget, char *label, 
				       char *pixmap, 
				       void (*func) (void *data), void *data);
/* creates an image widget of the file image at (x,y) of size (w x h) */
Epplet_gadget   Epplet_create_image(int x, int y, int w, int h, char *image);
/* puts a label widget of text label at (x,y) of size size. sizes are 0, 1 */
/* 2 and 3. 0 is normal , 1 is tiny, 2 is medium and 3 is big. experiment */
Epplet_gadget   Epplet_create_label(int x, int y, char *label, char size);
/* creates a horizontal progress bar at (x,y) of size (w x h) displaying the */
/* value val points to that is a value of 0-100. dir is the direction 0 */
/* indicates left to right and 1 indicates right to left */
Epplet_gadget   Epplet_create_hbar(int x, int y, int w, int h, char dir,
				   int *val);
/* creates a vertical progress bar - dir of 0 is top to bottom, 1 is bottom */
/* to top */
Epplet_gadget   Epplet_create_vbar(int x, int y, int w, int h, char dir,
				   int *val);
/* get the window id of the windopw to draw in in the drawing area */
Window          Epplet_get_drawingarea_window(Epplet_gadget gadget);
/* change the background to either verticla or horizontal for the epplet */
void            Epplet_background_properties(char vertical);
/* destroy a gadget */
void            Epplet_gadget_destroy(Epplet_gadget gadget);
/* hide a gadget */
void            Epplet_gadget_hide(Epplet_gadget gadget);
/* show a gadget */
void            Epplet_gadget_show(Epplet_gadget gadget);
/* if you cnaged the value a gadget is pointing to call this on the gadget */
/* so it can update and redraw */
void            Epplet_gadget_data_changed(Epplet_gadget gadget);
/* change the popup a popbutton brings up and destroy the popup it currently */
/* brings up */
void            Epplet_change_popbutton_popup(Epplet_gadget gadget, 
					      Epplet_gadget popup);
/* display the popup gadget above or below (dpeending where the window ww */
/* is) the window ww, or if it's 0, deisplay where the pointer is */
void            Epplet_pop_popup(Epplet_gadget gadget, Window ww);
/* change the image file and widht & height of the image gadget */
void            Epplet_change_image(Epplet_gadget gadget, int w, int h, 
				    char *image);
/* change the label string contents of the gadget label */
void            Epplet_change_label(Epplet_gadget gadget, char *label);


/****************************************************************************/
/* baisc line, filled rectangle and box outline drawing functions to make   */
/* life easy                                                                */
/****************************************************************************/
/* draw a line from (x1, y1) to (x2, y2) in window win, in color (r, g, b) */
void            Epplet_draw_line(Window win, int x1, int y1, int x2, int y2,
				 int r, int g, int b);
/* draw a box at (x, y) of size (w x h) in window win, in color (r, g, b) */
void            Epplet_draw_box(Window win, int x, int y, int w, int h,
				int r, int g, int b);
/* draw a box outline at (x, y) of size (w, h) in window win, in color (r, g, b) */
void            Epplet_draw_outline(Window win, int x, int y, int w, int h,
				    int r, int g, int b);
/* get the pixel value for the RGB value (r, g, b) and return it */
int             Epplet_get_color(int r, int g, int b);
/* pasye the image file image onto window ww, at its original size at (x,y) */
void            Epplet_paste_image(char *image, Window ww, int x, int y);
/* paste the image file image onto window ww at (x,y), at size (w x h) */
void            Epplet_paste_image_size(char *image, Window ww, int x, int y, 
					int w, int h);
/* syncronize all draws (guarantees they are done) */
void            Esync(void);

/****************************************************************************/
/* RGB buffer - for people who want to write raw RGB image data to a drawing*/
/* area and want it rendered/dithered etc. for them                         */
/****************************************************************************/
/* create an RGB buffer of size (w x h) */
RGB_buf         Epplet_make_rgb_buf(int w, int h);
/* get a pointer to the RGB data int he RGB buffer */
unsigned char  *Epplet_get_rgb_pointer(RGB_buf buf);
/* render & paste the RGB buffer to a window at x, y */
void            Epplet_paste_buf(RGB_buf buf, Window win, int x, int y);




/* command execution/spawing wrappers to make life easy */
void            Epplet_run_command(char *cmd);
char           *Epplet_read_run_command(char *cmd);
int             Epplet_spawn_command(char *cmd);
void            Epplet_pause_spawned_command(int pid);
void            Epplet_unpause_spawned_command(int pid);
void            Epplet_kill_spawned_command(int pid);
void            Epplet_destroy_spawned_command(int pid);
void            Epplet_register_child_handler(void (*func)
					      (void *data, int pid, int exit_code),
					      void *data);
void            Epplet_change_button_label(Epplet_gadget gadget, char *label);
void            Epplet_change_button_image(Epplet_gadget gadget, char *image);


/****************************************************************************/
/* Convenience macros to make using the above calls easier                  */
/****************************************************************************/
/* write a single RGB pixel in an RGB buffer. it isnt very efficient to call */
/* this lots - but it works well if called sparsely */
#define         Epplet_buf_write_pixel(buf, x, y, r, g, b) \
{\
   char *ptr;\
   ptr = (buf)->im->rgb_data + ((buf)->im->rgb_width * 3 * (w)) + (h);\
   ptr[0] = (unsigned char)(r);\
   ptr[1] = (unsigned char)(g);\
   ptr[2] = (unsigned char)(b);\
}

