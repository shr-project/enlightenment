#ifndef ELMXX_GEN_H
#define ELMXX_GEN_H

/* STL */
#include <string>

/* EFL */
#include <Elementary.h>

/* ELFxx */
#include "Object.h"

namespace Elmxx {

class Gen : public Object
{
public:  
  void clear ();

  void setAlwaysSelectMode (bool alwaysSelect);
  
  bool getAlwaysSelectMode ();
  
  void setNoSelectMode (bool noSelect);
  
  bool getNoSelectMode ();

  void setBounce (bool hBounce, bool vBounce);
  
  void getBounce (bool &hBounceOut, bool &vBounceOut);

protected:
  // allow only construction for child classes
  Gen (); // allow only construction for child classes
  virtual ~Gen (); // forbid direct delete -> use Object::destroy()

private:
  Gen (const Gen&); // forbid copy constructor

};

} // end namespace Elmxx

#endif // ELMXX_GEN_H
