#ifndef ELMXX_PANEL_H
#define ELMXX_PANEL_H

/* STL */
#include <string>

/* ELFxx */
#include "Object.h"

namespace Elmxx {

class Panel : public Object
{
public:
  static Panel *factory (Evasxx::Object &parent);

  void setOrientation (Elm_Panel_Orient orient);
  void setContent (Evasxx::Object &content);

protected:
  // allow only construction for child classes
  Panel (Evasxx::Object &parent); // private construction -> use factory ()
  virtual ~Panel (); // forbid direct delete -> use ElmWidget::destroy()
  
private:
  Panel (); // forbid standard constructor
  Panel (const Panel&); // forbid copy constructor
};

} // end namespace Elmxx

#endif // ELMXX_PANEL_H
