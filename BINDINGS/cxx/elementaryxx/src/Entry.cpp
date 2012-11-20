#ifdef HAVE_CONFIG_H
  #include <config.h>
#endif

#include "elementaryxx/Entry.h"

using namespace std;

namespace Elmxx {

Entry::Entry (Evasxx::Object &parent)
{
  o = elm_entry_add (parent.obj ());
  
  elmInit ();
}

Entry::~Entry () {}

Entry *Entry::factory (Evasxx::Object &parent)
{
  return new Entry (parent);
}

void Entry::pushUserTextStyle(const std::string &style)
{
  elm_entry_text_style_user_push(o, style.c_str());
}

void Entry::popUserTextStyle()
{
  elm_entry_text_style_user_pop(o);
}

std::string Entry::peekUserTextStyle()
{
  const char *tmp = elm_entry_text_style_user_peek(o);
  return tmp ? tmp : string ();
}

void Entry::setScrollable(bool scroll)
{
  elm_entry_scrollable_set(o, scroll);
}

bool Entry::getScrollable() const
{
  return elm_entry_scrollable_get(o);
}

void Entry::setSingleLine (bool single_line)
{
  elm_entry_single_line_set (o, single_line);
}

bool Entry::getSingleLine() const
{
  return elm_entry_single_line_get(o);
}

void Entry::setPassword (bool password)
{
  elm_entry_password_set (o, password);
}

bool Entry::getPassword() const
{
  return elm_entry_password_get(o);
}

void Entry::setText (const std::string &entry)
{
  elm_entry_entry_set (o, entry.c_str ());
}

std::string Entry::getText () const
{
  const char *tmp = elm_entry_entry_get (o);
  return tmp ? tmp : string ();
}

bool Entry::isEmpty() const
{
  return elm_entry_is_empty(o);
}

std::string Entry::getSelection () const
{
  const char *tmp = elm_entry_selection_get (o);
  return tmp ? tmp : string ();
}

void Entry::forceCalc()
{
  elm_entry_calc_force(o);
}

void Entry::insertText (const std::string &entry)
{
  elm_entry_entry_insert (o, entry.c_str ());
}

void Entry::appendText (const std::string &entry)
{
  // FIXME: hm, this doesn't link in application. Find out why...
  //elm_entry_entry_append(o, entry.c_str ());
}

void Entry::setLineWrap (Elm_Wrap_Type wrap)
{
  elm_entry_line_wrap_set (o, wrap);
}

Elm_Wrap_Type Entry::getLineWrap() const
{
  return elm_entry_line_wrap_get(o);
}

void Entry::setEditable (bool editable)
{
  elm_entry_editable_set (o, editable);
}

bool Entry::getEditable() const
{
  return elm_entry_editable_get(o);
}

void Entry::selectNone ()
{
  elm_entry_select_none (o);
}

void Entry::selectAll ()
{
  elm_entry_select_all (o);
}

bool Entry::cursorNext()
{
  return elm_entry_cursor_next(o);
}
    
bool Entry::cursorPrev()
{
  return elm_entry_cursor_prev(o);
}

bool Entry::cursorUp()
{  
  return elm_entry_cursor_up(o);
}

bool Entry::cursorDown()
{
  return elm_entry_cursor_down(o);
}
                
void Entry::setCursorBegin()
{
  elm_entry_cursor_begin_set(o);
}

void Entry::setCursorEnd()
{
  elm_entry_cursor_end_set(o);
}

void Entry::setCursorLineBegin()
{
  elm_entry_cursor_line_begin_set(o);
}

void Entry::setCursorLineEnd()
{
  elm_entry_cursor_line_end_set(o);
}

void Entry::beginCursorSelection()
{
  elm_entry_cursor_selection_begin(o);
}

void Entry::endCursorSelection()
{
  elm_entry_cursor_selection_end(o);
}

bool Entry::getCursorIsFormat() const
{
  return elm_entry_cursor_is_format_get(o);
}

bool Entry::getCursorIsVisibleFormat() const
{
  return elm_entry_cursor_is_visible_format_get(o);
}

void Entry::setCursorPos(int pos)
{
  elm_entry_cursor_pos_set(o, pos);
}

int Entry::getCursorPos() const
{
  return elm_entry_cursor_pos_get(o);
}

void Entry::cutSelection()
{
  elm_entry_selection_cut(o);
}

void Entry::copySelection()
{
  elm_entry_selection_copy(o);
}

void Entry::pasteSelection()
{
  elm_entry_selection_paste(o);
}

void Entry::clearContextMenu()
{
  elm_entry_context_menu_clear(o);
}

std::string Entry::markupToUtf8 (const std::string &s)
{
  char *tmp = elm_entry_markup_to_utf8 (s.c_str ());
  string ret (tmp ? tmp : string ());
  free (tmp);
  return ret;
}

std::string Entry::utf8ToMarkup (const std::string &s)
{
  char *tmp = elm_entry_utf8_to_markup (s.c_str ());
  string ret (tmp ? tmp : string ());
  free (tmp);
  return ret;
}

void Entry::setAnchorHoverParent(const Evasxx::Object &parent)
{
  elm_entry_anchor_hover_parent_set(o, parent.obj());
}

void Entry::setAnchorHoverStyle(const std::string &style)
{
  elm_entry_anchor_hover_style_set(o, style.c_str());
}

void Entry::setAnchorHoverDefaultStyle()
{
  elm_entry_anchor_hover_style_set(o, NULL);
}

void Entry::endAnchorHover()
{
  elm_entry_anchor_hover_end(o);
}

} // end namespace Elmxx
