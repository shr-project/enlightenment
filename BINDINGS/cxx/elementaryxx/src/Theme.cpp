#ifdef HAVE_CONFIG_H
  #include <config.h>
#endif

#include "../include/elementaryxx/Theme.h"

using namespace std;

namespace Elmxx {

Theme::Theme () :
  mTheme (NULL) // elm_theme_new ()
{
}

Theme::~Theme ()
{
  //elm_theme_free (mTheme);
}

Theme::Theme (const Theme &th)
{
  elm_theme_copy (th.mTheme, mTheme);
}

void Theme::addOverlay(const std::string& item)
{
  elm_theme_overlay_add(mTheme, item.c_str());
}

void Theme::delOverlay(const std::string& item)
{
  elm_theme_overlay_del(mTheme, item.c_str());   
}

void Theme::addExtension (const std::string& item)
{
  elm_theme_extension_add(mTheme, item.c_str());
}

void Theme::delExtension (const std::string& item)
{
  elm_theme_extension_del(mTheme, item.c_str());
}


} // end namespace Elmxx
