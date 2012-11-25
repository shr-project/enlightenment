#ifndef ELMXX_BUTTON_H
#define ELMXX_BUTTON_H

/* STL */
#include <string>

/* EFL */
#include <Elementary.h>

/* ELFxx */
#include "Object.h"

namespace Elmxx {

/*!
 * smart callbacks called:
 * "clicked" - the user clicked the button
 *
 * available styles: 
 * default 
 * hoversel_vertical 
 * hoversel_vertical_entry
 */
class Button : public Object
{
public:  
  static Button *factory (Evasxx::Object &parent);

protected:
  // allow only construction for child classes
  Button (Evasxx::Object &parent); // private construction -> use factory ()
  virtual ~Button (); // forbid direct delete -> use Object::destroy()
  
private:
  Button (); // forbid standard constructor
  Button (const Button&); // forbid copy constructor
};

} // end namespace Elmxx

#endif // ELMXX_BUTTON_H
