#ifndef ELMXX_ICON_H
#define ELMXX_ICON_H

/* STL */
#include <string>

/* EFL */
#include <Elementary.h>

/* ELFxx */
#include "Object.h"

namespace Elmxx {

/*!
 * smart callbacks called:
 * "clicked" - the user clicked the icon
 */
class Icon : public Object
{
public:
  static Icon *factory (Evasxx::Object &parent);

  bool setFile (const std::string &file);
  
  bool setFile (const std::string &file, const std::string &group);

  /**
   * Set the icon by icon standards names.
   *
   * @param obj The icon object
   * @param name The icon name
   *
   * @return (@c EINA_TRUE = success, @c EINA_FALSE = error)
   *
   * For example, freedesktop.org defines standard icon names such as "home",
   * "network", etc. There can be different icon sets to match those icon
   * keys. The @p name given as parameter is one of these "keys", and will be
   * used to look in the freedesktop.org paths and elementary theme. One can
   * change the lookup order with elm_icon_order_lookup_set().
   *
   * If name is not found in any of the expected locations and it is the
   * absolute path of an image file, this image will be used.
   *
   * @note The icon image set by this function can be changed by
   * elm_image_file_set().
   *
   * @see getStandard()
   * @see setFile()
   *
   * @ingroup Icon
   */
  void setStandard (const std::string &name);

  /**
   * Get the icon name set by icon standard names.
   *
   * @param obj The icon object
   * @return The icon name
   *
   * If the icon image was set using setFile() instead of
   * setStandard(), then this function will return @c NULL.
   *
   * @see setStandard()
   *
   * @ingroup Icon
   */
  //std::string getStandard();
  
  
  
  void setSmooth (bool smooth);

  void setNoScale (bool noScale);

  void setResizable (bool scaleUp, bool scaleDown);

  void setFillOutside (bool fillOutside);

  void setPrescale (int size);

protected:
  // allow only construction for child classes
  Icon (Evasxx::Object &parent); // private construction -> use factory ()
  ~Icon (); // forbid direct delete -> use Object::destroy()
  
private:
  Icon (); // forbid standard constructor
  Icon (const Icon&); // forbid copy constructor
};

} // end namespace Elmxx

#endif // ELMXX_ICON_H
