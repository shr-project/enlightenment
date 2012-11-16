#ifdef HAVE_CONFIG_H
  #include <config.h>
#endif

#include "../include/elementaryxx/Theme.h"

using namespace std;

namespace Elmxx {

Theme::Theme (bool default_theme)
{
  if (default_theme)
  {
    mTheme = NULL;
  }
  else
  {
    mTheme = elm_theme_new ();
  }
}

Theme::~Theme ()
{
  if (mTheme)
  {
    elm_theme_free (mTheme);
  }
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

Eflxx::CountedPtr <Einaxx::List<Theme*> > Theme::getOverlayList ()
{
  Eina_List *list = const_cast <Eina_List*> (elm_theme_overlay_list_get(mTheme));
  
  return Eflxx::CountedPtr <Einaxx::List<Theme*> > (Einaxx::List<Theme*>::wrap (list));
}  

void Theme::addExtension (const std::string& item)
{
  elm_theme_extension_add(mTheme, item.c_str());
}

void Theme::delExtension (const std::string& item)
{
  elm_theme_extension_del(mTheme, item.c_str());
}

Eflxx::CountedPtr <Einaxx::List<Theme*> > Theme::getExtensionList ()
{
  Eina_List *list = const_cast <Eina_List*> (elm_theme_extension_list_get(mTheme));
  
  return Eflxx::CountedPtr <Einaxx::List<Theme*> > (Einaxx::List<Theme*>::wrap (list));
}  

void Theme::setTheme (const std::string &theme)
{
  elm_theme_set(mTheme, theme.c_str());
}

std::string Theme::getTheme ()
{
  return elm_theme_get(mTheme);
}

std::string Theme::getItemListPath (const std::string &f, bool &in_search_path)
{
  Eina_Bool b;
  
  const char *ret = elm_theme_list_item_path_get(f.c_str(), &b);
  in_search_path = b;
  
  return ret;
}

Eflxx::CountedPtr <Einaxx::List<Theme*> > Theme::getThemeList ()
{
  Eina_List *list = const_cast <Eina_List*> (elm_theme_list_get(mTheme));
  
  return Eflxx::CountedPtr <Einaxx::List<Theme*> > (Einaxx::List<Theme*>::wrap (list));
}  

void Theme::flush ()
{
  elm_theme_flush(mTheme);
}

void Theme::flushFull ()
{
  elm_theme_full_flush();
}

std::string Theme::getData (const std::string &key)
{
  return elm_theme_data_get(mTheme, key.c_str());
}

} // end namespace Elmxx
