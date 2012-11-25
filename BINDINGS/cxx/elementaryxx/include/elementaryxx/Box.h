#ifndef ELMXX_BOX_H
#define ELMXX_BOX_H

/* STL */
#include <string>

/* EFL */
#include <Elementary.h>

/* ELFxx */
#include "Object.h"

namespace Elmxx {

class Box : public Object
{
public:
  static Box *factory (Evasxx::Object &parent);
  
  enum Orientation
  {
    Horizontal,
    Vertical
  };

  void setOrientation (Box::Orientation orient);
  
  void setHomogeneous (bool homogenous);

  void packStart (const Evasxx::Object &subobj);
  
  void packEnd (const Evasxx::Object &subobj);
  
  void packBefore (const Evasxx::Object &subobj, const Evasxx::Object &before);
  
  void packAfter (const Evasxx::Object &subobj, const Evasxx::Object &after);

protected:
  // allow only construction for child classes  
  Box (Evasxx::Object &parent); // private construction -> use factory ()  
  virtual ~Box (); // forbid direct delete -> use Object::destroy()
  
private:
  Box (); // forbid standard constructor
  Box (const Box&); // forbid copy constructor
};

} // end namespace Elmxx

#endif // ELMXX_BOX_H
