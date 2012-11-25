#ifndef ELMXX_HOVER_H
#define ELMXX_HOVER_H

/* STL */
#include <string>

/* ELFxx */
#include "Object.h"

namespace Elmxx {

/*! smart callbacks called:
 * "clicked" - the user clicked the empty space in the hover to dismiss
 *
 * available styles: 
 * default
 * popout
 * hoversel_vertical
 */
class Hover : public Object
{
public:
  static Hover *factory (Evasxx::Object &parent);

  void setTarget (const Evasxx::Object &target);

  void setParent (const Evasxx::Object &parent);

  const string getBestContentLocation (Elm_Hover_Axis prefAxis) const;

protected:
  // allow only construction for child classes
  Hover (Evasxx::Object &parent); // private construction -> use factory ()
  virtual ~Hover (); // forbid direct delete -> use Object::destroy()
  
private:
  Hover (); // forbid standard constructor
  Hover (const Hover&); // forbid copy constructor
};

} // end namespace Elmxx

#endif // ELMXX_HOVER_H
