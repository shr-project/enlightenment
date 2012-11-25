#ifndef ELMXX_BUBBLE_H
#define ELMXX_BUBBLE_H

/* STL */
#include <string>

/* EFL */
#include <Elementary.h>

/* ELFxx */
#include "Object.h"

namespace Elmxx {

class Bubble : public Object
{
public:
  static Bubble *factory (Evasxx::Object &parent);
  
  void setCorner (const std::string &corner);

protected:
  // allow only construction for child classes
  Bubble (Evasxx::Object &parent); // private construction -> use factory ()
  virtual ~Bubble (); // forbid direct delete -> use Object::destroy()  
  
private:
  Bubble (); // forbid standard constructor
  Bubble (const Bubble&); // forbid copy constructor
};

} // end namespace Elmxx

#endif // ELMXX_BUBBLE_H
