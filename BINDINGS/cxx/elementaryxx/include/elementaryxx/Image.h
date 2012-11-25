#ifndef ELMXX_IMAGE_H
#define ELMXX_IMAGE_H

/* STL */
#include <string>

/* EFL */
#include <Elementary.h>

/* ELFxx */
#include "Object.h"

namespace Elmxx {

/*!
 * smart callbacks called:
 * "clicked" - the user clicked the image
 */
class Image : public Object
{
public:
  static Image *factory (Evasxx::Object &parent);

  bool setFile (const std::string &file, const std::string &group);
  
  void setSmooth (bool smooth);
  
  void setNoScale (bool noScale);
  
  void setResizable (bool scaleUp, bool scaleDown);
  
  void setFillOutside (bool fillOutside);
  
  void setPrescale (int size);
  
  void setOrient (Elm_Image_Orient orient);

protected:
  Image (Evasxx::Object &parent); // private construction -> use factory ()
  virtual ~Image (); // forbid direct delete -> use Object::destroy()
  
private:
  Image (); // forbid standard constructor
  Image (const Image&); // forbid copy constructor
};

} // end namespace Elmxx

#endif // ELMXX_IMAGE_H
