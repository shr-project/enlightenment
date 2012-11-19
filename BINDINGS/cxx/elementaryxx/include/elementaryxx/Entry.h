#ifndef ELMXX_ENTRY_H
#define ELMXX_ENTRY_H

/* STL */
#include <string>

/* EFL */
#include <Elementary.h>

/* ELFxx */
#include "Object.h"

namespace Elmxx {

/*!
 * smart callbacks called:
 * "changed" - the text content changed
 * "selection,start" - the user started selecting text
 * "selection,changed" - the user modified the selection size/location
 * "selection,cleared" - the user cleared the selection
 * "selection,paste" - the user rrequested a paste of text
 * "selection,copy" - the user copied the text
 * "selection,cut" - the user cut the text
 * "cursor,changed" - the cursor changed position
 * "anchor,clicked" - achor called was clicked | event_info = Elm_Entry_Anchor_Info
 * "activated" - when the enter key is pressed (useful for single line)
 */
  // TODO: implement ScrolledEntry with elm_scrolled_entry_add()...
class Entry : public Object
{
public:
  static Entry *factory (Evasxx::Object &parent);

  /**
   * Push the style to the top of user style stack.
   * If there is styles in the user style stack, the properties in the top style
   * of user style stack will replace the properties in current theme.
   * The input style is specified in format tag='property=value' (i.e. DEFAULT='font=Sans font_size=60'hilight=' + font_weight=Bold').
   *
   * @param style The style user to push
   */
  void pushUserTextStyle(const std::string &style);

  /**
   * Remove the style in the top of user style stack.
   *
   * @see pushUserTextStyle()
   */
  void popUserTextStyle();

  /**
   * Retrieve the style on the top of user style stack.
   *
   * @return style on the top of user style stack if exist, otherwise NULL.
   
   * @see pushUserTextStyle()
   */
  std::string peekUserTextStyle(); 

  /**
   * Enable or disable scrolling in entry
   *
   * Normally the entry is not scrollable unless you enable it with this call.
   *
   * @param scroll true if it is to be scrollable, false otherwise
   *
   * @ingroup Entry
   */
  void setScrollable(bool scroll);

  /**
   * Get the scrollable state of the entry
   *
   * Normally the entry is not scrollable. This gets the scrollable state
   * of the entry. See setScrollable() for more information.
   *
   * @return The scrollable state
   *
   * @ingroup Entry
   */
  bool getScrollable() const;

  /**
   * Sets the entry to single line mode.
   *
   * In single line mode, entries don't ever wrap when the text reaches the
   * edge, and instead they keep growing horizontally. Pressing the @c Enter
   * key will generate an @c "activate" event instead of adding a new line.
   *
   * When @p single_line is @c false, line wrapping takes effect again
   * and pressing enter will break the text into a different line
   * without generating any events.
   *
   * @param single_line If true, the text in the entry
   * will be on a single line.
   *
   * @ingroup Entry
   */
  void setSingleLine(bool single_line);

  /**
   * Gets whether the entry is set to be single line.
   *
   * @return single_line If true, the text in the entry is set to display
   * on a single line.
   *
   * @see setSingleLine()
   *
   * @ingroup Entry
   */
  bool getSingleLine() const;

  /**
   * Sets the entry to password mode.
   *
   * In password mode, entries are implicitly single line and the display of
   * any text in them is replaced with asterisks (*).
   *
   * @param password If true, password mode is enabled.
   *
   * @ingroup Entry
   */
  void setPassword (bool password);

  /**
   * Gets whether the entry is set to password mode.
   *
   * @return If true, the entry is set to display all characters
   * as asterisks (*).
   *
   * @see setPassword()
   *
   * @ingroup Entry
   */
  bool getPassword() const;

  /**
   * This sets the text displayed within the entry to @p entry.
   *
   * @param entry The text to be displayed
   *
   * @note Using this function bypasses text filters
   *
   * @ingroup Entry
   */
  void setText (const std::string &entry);
  

  /**
   * This returns the text currently shown in object @p entry.
   * See also setText().
   *
   * @return The currently displayed text
   *
   * @ingroup Entry
   */
  std::string getText () const;

  /**
   * Gets whether the entry is empty.
   *
   * Empty means no text at all. If there are any markup tags, like an item
   * tag for which no provider finds anything, and no text is displayed, this
   * function still returns false.
   *
   * @return true if the entry is empty, false otherwise.
   *
   * @ingroup Entry
   */
  bool isEmpty() const;

  /**
   * Gets any selected text within the entry.
   *
   * If there's any selected text in the entry, this function returns it as
   * a string in markup format. NULL is returned if no selection exists or
   * if an error occurred.
   *
   * The returned value points to an internal string and should not be freed
   * or modified in any way. If the @p entry object is deleted or its
   * contents are changed, the returned pointer should be considered invalid.
   *
   * @return The selected text within the entry or NULL on failure
   *
   * @ingroup Entry
   */
  std::string getSelection () const;

  /**
   * Forces calculation of the entry size and text layouting.
   *
   * This should be used after modifying the textblock object directly. See
   * getTextblock() for more information.
   *
   *
   * @see getTextblock()
   *
   * @ingroup Entry
   */
  void forceCalc();

  /**
   * Inserts the given text into the entry at the current cursor position.
   *
   * This inserts text at the cursor position as if it was typed
   * by the user (note that this also allows markup which a user
   * can't just "type" as it would be converted to escaped text, so this
   * call can be used to insert things like emoticon items or bold push/pop
   * tags, other font and color change tags etc.)
   *
   * If any selection exists, it will be replaced by the inserted text.
   *
   * The inserted text is subject to any filters set for the widget.
   *
   * @param entry The text to insert
   *
   * @see appendMarkupFilter()
   *
   * @ingroup Entry
   */
  void insertText (const std::string &entry);

  /**
   * Appends @p entry to the text of the entry.
   *
   * Adds the text in @p entry to the end of any text already present in the
   * widget.
   *
   * The appended text is subject to any filters set for the widget.
   *
   * @param entry The text to be displayed
   *
   * @see appendMarkupFilter()
   *
   * @ingroup Entry
   */
  void appendText (const std::string &entry);
  EAPI void               elm_entry_entry_append(Evas_Object *obj, const char *entry);

  /**
   * Set the line wrap type to use on multi-line entries.
   *
   * Sets the wrap type used by the entry to any of the specified in
   * Elm_Wrap_Type. This tells how the text will be implicitly cut into a new
   * line (without inserting a line break or paragraph separator) when it
   * reaches the far edge of the widget.
   *
   * Note that this only makes sense for multi-line entries. A widget set
   * to be single line will never wrap.
   *
   * @param wrap The wrap mode to use. See Elm_Wrap_Type for details on them
   */
  void setLineWrap (Elm_Wrap_Type wrap);

  /**
   * Gets the wrap mode the entry was set to use.
   *
   * @return Wrap type
   *
   * @see also setLineWrap()
   *
   * @ingroup Entry
   */
  Elm_Wrap_Type getLineWrap() const;

  /**
   * Sets if the entry is to be editable or not.
   *
   * By default, entries are editable and when focused, any text input by the
   * user will be inserted at the current cursor position. But calling this
   * function with @p editable as false will prevent the user from
   * inputting text into the entry.
   *
   * The only way to change the text of a non-editable entry is to use
   * setText() and other related functions.
   *
   * @param editable If true, user input will be inserted in the entry,
   * if not, the entry is read-only and no user input is allowed.
   *
   * @ingroup Entry
   */
  void setEditable (bool editable);

  /**
   * Gets whether the entry is editable or not.
   *
   * @return If true, the entry is editable by the user.
   * If false, it is not editable by the user
   *
   * @see setEditable()
   *
   * @ingroup Entry
   */
  bool getEditable() const;

  /**
   * This drops any existing text selection within the entry.
   *
   * @ingroup Entry
   */
  void selectNone ();

  /**
   * This selects all text within the entry.
   *
   * @ingroup Entry
   */
  void selectAll ();

  /**
   * This moves the cursor one place to the right within the entry.
   *
   * @return true upon success, false upon failure
   *
   * @ingroup Entry
   */
  bool cursorNext();

  /**
   * This moves the cursor one place to the left within the entry.
   *
   * @return true upon success, false upon failure
   *
   * @ingroup Entry
   */
  bool cursorPrev();

  /**
   * This moves the cursor one line up within the entry.
   *
   * @return true upon success, false upon failure
   *
   * @ingroup Entry
   */
  bool cursorUp();

  /**
   * This moves the cursor one line down within the entry.
   *
   * @return true upon success, false upon failure
   *
   * @ingroup Entry
   */
  bool cursorDown();

  /**
   * This moves the cursor to the beginning of the entry.
   *
   * @ingroup Entry
   */
  void setCursorBegin();

  /**
   * This moves the cursor to the end of the entry.
   *
   * @ingroup Entry
   */
  void setCursorEnd();

  /**
   * This moves the cursor to the beginning of the current line.
   *
   * @ingroup Entry
   */
  void setCursorLineBegin();

  /**
   * This moves the cursor to the end of the current line.
   *
   * @ingroup Entry
   */
  void setCursorLineEnd();

  /**
   * This begins a selection within the entry as though
   * the user were holding down the mouse button to make a selection.
   *
   * @ingroup Entry
   */
  void beginCursorSelection();

  /**
   * This ends a selection within the entry as though
   * the user had just released the mouse button while making a selection.
   *
   * @ingroup Entry
   */
  void endCursorSelection();

  /**
   * Gets whether a format node exists at the current cursor position.
   *
   * A format node is anything that defines how the text is rendered. It can
   * be a visible format node, such as a line break or a paragraph separator,
   * or an invisible one, such as bold begin or end tag.
   * This function returns whether any format node exists at the current
   * cursor position.
   *
   * @return true if the current cursor position contains a format node,
   * false otherwise.
   *
   * @see getCursorIsVisibleFormat()
   *
   * @ingroup Entry
   */
  bool getCursorIsFormat() const;

  /**
   * Gets if the current cursor position holds a visible format node.
   *
   * @return true if the current cursor is a visible format, false
   * if it's an invisible one or no format exists.
   *
   * @see getCursorIsFormat()
   *
   * @ingroup Entry
   */
  bool getCursorIsVisibleFormat() const;

  /**
   * Sets the cursor position in the entry to the given value
   *
   * The value in @p pos is the index of the character position within the
   * contents of the string as returned by elm_entry_cursor_pos_get().
   *
   * @param pos The position of the cursor
   *
   * @ingroup Entry
   */
  void setCursorPos(int pos);

  /**
   * Retrieves the current position of the cursor in the entry
   *
   * @return The cursor position
   *
   * @ingroup Entry
   */
  int getCursorPos() const;

  /**
   * This executes a "cut" action on the selected text in the entry.
   *
   * @ingroup Entry
   */
  void cutSelection();

  /**
   * This executes a "copy" action on the selected text in the entry.
   *
   * @ingroup Entry
   */
  void copySelection();

  /**
   * This executes a "paste" action in the entry.
   *
   * @ingroup Entry
   */
  void pasteSelection();

  /**
   * This clears and frees the items in a entry's contextual (longpress)
   * menu.
   *
   * @see addContextMenuItem()
   *
   * @ingroup Entry
   */
  void clearContextMenu();

  /**
   * This converts a markup (HTML-like) string into UTF-8.
   *
   * @param s The string (in markup) to be converted
   * @return The converted string (in UTF-8).
   *
   * @ingroup Entry
   */
  static std::string markupToUtf8(const std::string &s);

  /**
   * This converts a UTF-8 string into markup (HTML-like).
   *
   * @param s The string (in UTF-8) to be converted
   * @return The converted string (in markup).
   *
   * @ingroup Entry
   */
  static std::string utf8ToMarkup(const std::string &s);

  /**
   * Set the parent of the hover popup
   *
   * Sets the parent object to use by the hover created by the entry
   * when an anchor is clicked. See @ref Hover for more details on this.
   *
   * @param parent The object to use as parent for the hover
   *
   * @ingroup Entry
   */
  void setAnchorHoverParent(const Evasxx::Object &parent);
  
  /**
   * Get the parent of the hover popup
   *
   * Get the object used as parent for the hover created by the entry
   * widget. See @ref Hover for more details on this.
   * If no parent is set, the same entry object will be used.
   *
   * @return The object used as parent for the hover, NULL if none is set.
   *
   * @ingroup Entry
   */
  // FIXME
  //EAPI Evas_Object                *elm_entry_anchor_hover_parent_get(const Evas_Object *obj);

  /**
   * Set the style that the hover should use
   *
   * When creating the popup hover, entry will request that it's
   * themed according to @p style.
   *
   * Setting style no @c NULL means disabling automatic hover.
   *
   * @param style The style to use for the underlying hover
   *
   * @see setStyle()
   *
   * @ingroup Entry
   */
  void setAnchorHoverStyle(const std::string &style);

  /**
   * Set the style that the hover should use
   *
   * When creating the popup hover, entry will request that it's
   * tdisabling automatic hover.
   *
   * @see setStyle()
   *
   * @ingroup Entry
   */
  void setAnchorHoverDefaultStyle();

  /**
   * Get the style that the hover should use
   *
   * Get the style, the hover created by entry will use.
   *
   * @return The style to use by the hover. @c NULL means the default is used.
   *
   * @see elm_object_style_set()
   *
   * @ingroup Entry
   */
  // FIXME
  //EAPI const char                 *elm_entry_anchor_hover_style_get(const Evas_Object *obj);

  /**
   * Ends the hover popup in the entry
   *
   * When an anchor is clicked, the entry widget will create a hover
   * object to use as a popup with user provided content. This function
   * terminates this popup, returning the entry to its normal state.
   *
   * @ingroup Entry
   */
  void endAnchorHover();
  
private:
  Entry (); // forbid standard constructor
  Entry (const Entry&); // forbid copy constructor
  Entry (Evasxx::Object &parent); // private construction -> use factory ()
  ~Entry (); // forbid direct delete -> use Object::destroy()
};

#if 0


/**
 * Returns the actual textblock object of the entry.
 *
 * This function exposes the internal textblock object that actually
 * contains and draws the text. This should be used for low-level
 * manipulations that are otherwise not possible.
 *
 * Changing the textblock directly from here will not notify edje/elm to
 * recalculate the textblock size automatically, so any modifications
 * done to the textblock returned by this function should be followed by
 * a call to elm_entry_calc_force().
 *
 * The return value is marked as const as an additional warning.
 * One should not use the returned object with any of the generic evas
 * functions (geometry_get/resize/move and etc), but only with the textblock
 * functions; The former will either not work at all, or break the correct
 * functionality.
 *
 * IMPORTANT: Many functions may change (i.e delete and create a new one)
 * the internal textblock object. Do NOT cache the returned object, and try
 * not to mix calls on this object with regular elm_entry calls (which may
 * change the internal textblock object). This applies to all cursors
 * returned from textblock calls, and all the other derivative values.
 *
 * @param obj The entry object
 * @return The textblock object.
 *
 * @ingroup Entry
 */
EAPI Evas_Object *      elm_entry_textblock_get(Evas_Object *obj);








/**
 * Gets the character pointed by the cursor at its current position.
 *
 * This function returns a string with the utf8 character stored at the
 * current cursor position.
 * Only the text is returned, any format that may exist will not be part
 * of the return value. You must free the string when done with free().
 *
 * @param obj The entry object
 * @return The text pointed by the cursors.
 *
 * @ingroup Entry
 */
EAPI char              *elm_entry_cursor_content_get(const Evas_Object *obj);

/**
 * This function returns the geometry of the cursor.
 *
 * It's useful if you want to draw something on the cursor (or where it is),
 * or for example in the case of scrolled entry where you want to show the
 * cursor.
 *
 * @param obj The entry object
 * @param x returned geometry
 * @param y returned geometry
 * @param w returned geometry
 * @param h returned geometry
 * @return EINA_TRUE upon success, EINA_FALSE upon failure
 *
 * @ingroup Entry
 */
EAPI Eina_Bool          elm_entry_cursor_geometry_get(const Evas_Object *obj, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h);



/**
 * This adds an item to the entry's contextual menu.
 *
 * A longpress on an entry will make the contextual menu show up, if this
 * hasn't been disabled with elm_entry_context_menu_disabled_set().
 * By default, this menu provides a few options like enabling selection mode,
 * which is useful on embedded devices that need to be explicit about it,
 * and when a selection exists it also shows the copy and cut actions.
 *
 * With this function, developers can add other options to this menu to
 * perform any action they deem necessary.
 *
 * @param obj The entry object
 * @param label The item's text label
 * @param icon_file The item's icon file
 * @param icon_type The item's icon type
 * @param func The callback to execute when the item is clicked
 * @param data The data to associate with the item for related functions
 *
 * @ingroup Entry
 */
EAPI void               elm_entry_context_menu_item_add(Evas_Object *obj, const char *label, const char *icon_file, Elm_Icon_Type icon_type, Evas_Smart_Cb func, const void *data);

/**
 * This disables the entry's contextual (longpress) menu.
 *
 * @param obj The entry object
 * @param disabled If true, the menu is disabled
 *
 * @ingroup Entry
 */
EAPI void               elm_entry_context_menu_disabled_set(Evas_Object *obj, Eina_Bool disabled);

/**
 * This returns whether the entry's contextual (longpress) menu is
 * disabled.
 *
 * @param obj The entry object
 * @return If true, the menu is disabled
 *
 * @ingroup Entry
 */
EAPI Eina_Bool          elm_entry_context_menu_disabled_get(const Evas_Object *obj);

/**
 * This appends a custom item provider to the list for that entry
 *
 * This appends the given callback. The list is walked from beginning to end
 * with each function called given the item href string in the text. If the
 * function returns an object handle other than NULL (it should create an
 * object to do this), then this object is used to replace that item. If
 * not the next provider is called until one provides an item object, or the
 * default provider in entry does.
 *
 * @param obj The entry object
 * @param func The function called to provide the item object
 * @param data The data passed to @p func
 *
 * @see @ref entry-items
 *
 * @ingroup Entry
 */
EAPI void               elm_entry_item_provider_append(Evas_Object *obj, Elm_Entry_Item_Provider_Cb func, void *data);

/**
 * This prepends a custom item provider to the list for that entry
 *
 * This prepends the given callback. See elm_entry_item_provider_append() for
 * more information
 *
 * @param obj The entry object
 * @param func The function called to provide the item object
 * @param data The data passed to @p func
 *
 * @ingroup Entry
 */
EAPI void               elm_entry_item_provider_prepend(Evas_Object *obj, Elm_Entry_Item_Provider_Cb func, void *data);

/**
 * This removes a custom item provider to the list for that entry
 *
 * This removes the given callback. See elm_entry_item_provider_append() for
 * more information
 *
 * @param obj The entry object
 * @param func The function called to provide the item object
 * @param data The data passed to @p func
 *
 * @ingroup Entry
 */
EAPI void               elm_entry_item_provider_remove(Evas_Object *obj, Elm_Entry_Item_Provider_Cb func, void *data);

/**
 * Append a markup filter function for text inserted in the entry
 *
 * Append the given callback to the list. This functions will be called
 * whenever any text is inserted into the entry, with the text to be inserted
 * as a parameter. The type of given text is always markup.
 * The callback function is free to alter the text in any way it wants, but
 * it must remember to free the given pointer and update it.
 * If the new text is to be discarded, the function can free it and set its
 * text parameter to NULL. This will also prevent any following filters from
 * being called.
 *
 * @param obj The entry object
 * @param func The function to use as text filter
 * @param data User data to pass to @p func
 *
 * @ingroup Entry
 */
EAPI void               elm_entry_markup_filter_append(Evas_Object *obj, Elm_Entry_Filter_Cb func, void *data);

/**
 * Prepend a markup filter function for text inserted in the entry
 *
 * Prepend the given callback to the list. See elm_entry_markup_filter_append()
 * for more information
 *
 * @param obj The entry object
 * @param func The function to use as text filter
 * @param data User data to pass to @p func
 *
 * @ingroup Entry
 */
EAPI void               elm_entry_markup_filter_prepend(Evas_Object *obj, Elm_Entry_Filter_Cb func, void *data);

/**
 * Remove a markup filter from the list
 *
 * Removes the given callback from the filter list. See
 * elm_entry_markup_filter_append() for more information.
 *
 * @param obj The entry object
 * @param func The filter function to remove
 * @param data The user data passed when adding the function
 *
 * @ingroup Entry
 */
EAPI void               elm_entry_markup_filter_remove(Evas_Object *obj, Elm_Entry_Filter_Cb func, void *data);



/**
 * This sets the file (and implicitly loads it) for the text to display and
 * then edit. All changes are written back to the file after a short delay if
 * the entry object is set to autosave (which is the default).
 *
 * If the entry had any other file set previously, any changes made to it
 * will be saved if the autosave feature is enabled, otherwise, the file
 * will be silently discarded and any non-saved changes will be lost.
 *
 * @param obj The entry object
 * @param file The path to the file to load and save
 * @param format The file format
 * @return @c EINA_TRUE on success, @c EINA_FALSE otherwise
 *
 * @ingroup Entry
 */
EAPI Eina_Bool          elm_entry_file_set(Evas_Object *obj, const char *file, Elm_Text_Format format);

/**
 * Gets the file being edited by the entry.
 *
 * This function can be used to retrieve any file set on the entry for
 * edition, along with the format used to load and save it.
 *
 * @param obj The entry object
 * @param file The path to the file to load and save
 * @param format The file format
 *
 * @ingroup Entry
 */
EAPI void               elm_entry_file_get(const Evas_Object *obj, const char **file, Elm_Text_Format *format);

/**
 * This function writes any changes made to the file set with
 * elm_entry_file_set()
 *
 * @param obj The entry object
 *
 * @ingroup Entry
 */
EAPI void               elm_entry_file_save(Evas_Object *obj);

/**
 * This sets the entry object to 'autosave' the loaded text file or not.
 *
 * @param obj The entry object
 * @param autosave Autosave the loaded file or not
 *
 * @see elm_entry_file_set()
 *
 * @ingroup Entry
 */
EAPI void               elm_entry_autosave_set(Evas_Object *obj, Eina_Bool autosave);

/**
 * This gets the entry object's 'autosave' status.
 *
 * @param obj The entry object
 * @return Autosave the loaded file or not
 *
 * @see elm_entry_file_set()
 *
 * @ingroup Entry
 */
EAPI Eina_Bool          elm_entry_autosave_get(const Evas_Object *obj);

//scrollable...

/**
 * Sets the visibility of the left-side widget of the entry,
 * set by elm_object_part_content_set().
 *
 * @param obj The entry object
 * @param setting EINA_TRUE if the object should be displayed,
 * EINA_FALSE if not.
 *
 * @ingroup Entry
 */
EAPI void               elm_entry_icon_visible_set(Evas_Object *obj, Eina_Bool setting);

/**
 * Sets the visibility of the end widget of the entry, set by
 * elm_object_part_content_set(ent, "end", content).
 *
 * @param obj The entry object
 * @param setting EINA_TRUE if the object should be displayed,
 * EINA_FALSE if not.
 *
 * @ingroup Entry
 */
EAPI void               elm_entry_end_visible_set(Evas_Object *obj, Eina_Bool setting);

/**
 * This sets the entry's scrollbar policy (i.e. enabling/disabling
 * them).
 *
 * Setting an entry to single-line mode with elm_entry_single_line_set()
 * will automatically disable the display of scrollbars when the entry
 * moves inside its scroller.
 *
 * @param obj The entry object
 * @param h The horizontal scrollbar policy to apply
 * @param v The vertical scrollbar policy to apply
 *
 * @deprecated Use elm_scroller_policy_set() instead.
 *
 * @ingroup Entry
 */
EINA_DEPRECATED EAPI void elm_entry_scrollbar_policy_set(Evas_Object *obj, Elm_Scroller_Policy h, Elm_Scroller_Policy v);

/**
 * This enables/disables bouncing within the entry.
 *
 * This function sets whether the entry will bounce when scrolling reaches
 * the end of the contained entry.
 *
 * @param obj The entry object
 * @param h_bounce The horizontal bounce state
 * @param v_bounce The vertical bounce state
 *
 * @deprecated Use elm_scroller_bounce_set() instead.
 *
 * @ingroup Entry
 */
EINA_DEPRECATED EAPI void elm_entry_bounce_set(Evas_Object *obj, Eina_Bool h_bounce, Eina_Bool v_bounce);

/**
 * Get the bounce mode
 *
 * @param obj The Entry object
 * @param h_bounce Allow bounce horizontally
 * @param v_bounce Allow bounce vertically
 *
 * @deprecated Use elm_scroller_bounce_get() instead.
 *
 * @ingroup Entry
 */
EINA_DEPRECATED EAPI void elm_entry_bounce_get(const Evas_Object *obj, Eina_Bool *h_bounce, Eina_Bool *v_bounce);

/**
 * Set the input panel layout of the entry
 *
 * @param obj The entry object
 * @param layout layout type
 *
 * @ingroup Entry
 */
EAPI void                   elm_entry_input_panel_layout_set(Evas_Object *obj, Elm_Input_Panel_Layout layout);

/**
 * Get the input panel layout of the entry
 *
 * @param obj The entry object
 * @return layout type
 *
 * @see elm_entry_input_panel_layout_set
 *
 * @ingroup Entry
 */
EAPI Elm_Input_Panel_Layout elm_entry_input_panel_layout_get(const Evas_Object *obj);

/**
 * Set the autocapitalization type on the immodule.
 *
 * @param obj The entry object
 * @param autocapital_type The type of autocapitalization
 *
 * @ingroup Entry
 */
EAPI void                   elm_entry_autocapital_type_set(Evas_Object *obj, Elm_Autocapital_Type autocapital_type);

/**
 * Retrieve the autocapitalization type on the immodule.
 *
 * @param obj The entry object
 * @return autocapitalization type
 *
 * @ingroup Entry
 */
EAPI Elm_Autocapital_Type   elm_entry_autocapital_type_get(const Evas_Object *obj);

/**
 * Sets the attribute to show the input panel automatically.
 *
 * @param obj The entry object
 * @param enabled If true, the input panel is appeared when entry is clicked or has a focus
 *
 * @ingroup Entry
 */
EAPI void                   elm_entry_input_panel_enabled_set(Evas_Object *obj, Eina_Bool enabled);

/**
 * Retrieve the attribute to show the input panel automatically.
 *
 * @param obj The entry object
 * @return EINA_TRUE if input panel will be appeared when the entry is clicked or has a focus, EINA_FALSE otherwise
 *
 * @ingroup Entry
 */
EAPI Eina_Bool              elm_entry_input_panel_enabled_get(const Evas_Object *obj);

/**
 * Show the input panel (virtual keyboard) based on the input panel property of entry such as layout, autocapital types, and so on.
 *
 * Note that input panel is shown or hidden automatically according to the focus state of entry widget.
 * This API can be used in the case of manually controlling by using elm_entry_input_panel_enabled_set(en, EINA_FALSE).
 *
 * @param obj The entry object
 *
 * @ingroup Entry
 */
EAPI void                   elm_entry_input_panel_show(Evas_Object *obj);

/**
 * Hide the input panel (virtual keyboard).
 *
 * Note that input panel is shown or hidden automatically according to the focus state of entry widget.
 * This API can be used in the case of manually controlling by using elm_entry_input_panel_enabled_set(en, EINA_FALSE)
 *
 * @param obj The entry object
 *
 * @ingroup Entry
 */
EAPI void                   elm_entry_input_panel_hide(Evas_Object *obj);

/**
 * Set the language mode of the input panel.
 *
 * This API can be used if you want to show the alphabet keyboard mode.
 *
 * @param obj The entry object
 * @param lang language to be set to the input panel.
 *
 * @ingroup Entry
 */
EAPI void                   elm_entry_input_panel_language_set(Evas_Object *obj, Elm_Input_Panel_Lang lang);

/**
 * Get the language mode of the input panel.
 *
 * See @ref elm_entry_input_panel_language_set for more details.
 *
 * @param obj The entry object
 * @return input panel language type
 *
 * @ingroup Entry
 */
EAPI Elm_Input_Panel_Lang   elm_entry_input_panel_language_get(const Evas_Object *obj);

/**
 * Set the input panel-specific data to deliver to the input panel.
 *
 * This API is used by applications to deliver specific data to the input panel.
 * The data format MUST be negotiated by both application and the input panel.
 * The size and format of data are defined by the input panel.
 *
 * @param obj The entry object
 * @param data The specific data to be set to the input panel.
 * @param len the length of data, in bytes, to send to the input panel
 *
 * @ingroup Entry
 */
EAPI void                   elm_entry_input_panel_imdata_set(Evas_Object *obj, const void *data, int len);

/**
 * Get the specific data of the current input panel.
 *
 * See @ref elm_entry_input_panel_imdata_set for more details.
 *
 * @param obj The entry object
 * @param data The specific data to be got from the input panel
 * @param len The length of data
 *
 * @ingroup Entry
 */
EAPI void                   elm_entry_input_panel_imdata_get(const Evas_Object *obj, void *data, int *len);

/**
 * Set the "return" key type. This type is used to set string or icon on the "return" key of the input panel.
 *
 * An input panel displays the string or icon associated with this type
 *
 * @param obj The entry object
 * @param return_key_type The type of "return" key on the input panel
 *
 * @ingroup Entry
 */
EAPI void                   elm_entry_input_panel_return_key_type_set(Evas_Object *obj, Elm_Input_Panel_Return_Key_Type return_key_type);

/**
 * Get the "return" key type.
 *
 * @see elm_entry_input_panel_return_key_type_set() for more details
 *
 * @param obj The entry object
 * @return The type of "return" key on the input panel
 *
 * @ingroup Entry
 */
EAPI Elm_Input_Panel_Return_Key_Type elm_entry_input_panel_return_key_type_get(const Evas_Object *obj);

/**
 * Set the return key on the input panel to be disabled.
 *
 * @param obj The entry object
 * @param disabled The state to put in in: @c EINA_TRUE for
 *        disabled, @c EINA_FALSE for enabled
 *
 * @ingroup Entry
 */
EAPI void                   elm_entry_input_panel_return_key_disabled_set(Evas_Object *obj, Eina_Bool disabled);

/**
 * Get whether the return key on the input panel should be disabled or not.
 *
 * @param obj The entry object
 * @return EINA_TRUE if it should be disabled
 *
 * @ingroup Entry
 */
EAPI Eina_Bool              elm_entry_input_panel_return_key_disabled_get(const Evas_Object *obj);

/**
 * Set whether the return key on the input panel is disabled automatically when entry has no text.
 *
 * If @p enabled is EINA_TRUE, The return key on input panel is disabled when the entry has no text.
 * The return key on the input panel is automatically enabled when the entry has text.
 * The default value is EINA_FALSE.
 *
 * @param obj The entry object
 * @param enabled If @p enabled is EINA_TRUE, the return key is automatically disabled when the entry has no text.
 *
 * @ingroup Entry
 */
EAPI void                   elm_entry_input_panel_return_key_autoenabled_set(Evas_Object *obj, Eina_Bool enabled);

/**
 * Reset the input method context of the entry if needed.
 *
 * This can be necessary in the case where modifying the buffer would confuse on-going input method behavior.
 * This will typically cause the Input Method Context to clear the preedit state.
 * @param obj The entry object
 *
 * @ingroup Entry
 */
EAPI void                   elm_entry_imf_context_reset(Evas_Object *obj);

/**
 * Set whether the entry should allow to use the text prediction.
 *
 * @param obj The entry object
 * @param prediction Whether the entry should allow to use the text prediction.
 *
 * @ingroup Entry
 */
EAPI void                   elm_entry_prediction_allow_set(Evas_Object *obj, Eina_Bool prediction);

/**
 * Get whether the entry should allow to use the text prediction.
 *
 * @param obj The entry object
 * @return EINA_TRUE if it allows to use the text prediction, otherwise EINA_FALSE.
 *
 * @ingroup Entry
 */
EAPI Eina_Bool              elm_entry_prediction_allow_get(const Evas_Object *obj);

/* pre-made filters for entries */

/**
 * @typedef Elm_Entry_Filter_Limit_Size
 *
 * Data for the elm_entry_filter_limit_size() entry filter.
 */
typedef struct _Elm_Entry_Filter_Limit_Size Elm_Entry_Filter_Limit_Size;

/**
 * @struct _Elm_Entry_Filter_Limit_Size
 *
 * Data for the elm_entry_filter_limit_size() entry filter.
 */
struct _Elm_Entry_Filter_Limit_Size
{
   int max_char_count;      /**< The maximum number of characters allowed. */
   int max_byte_count;      /**< The maximum number of bytes allowed*/
};

/**
 * Filter inserted text based on user defined character and byte limits
 *
 * Add this filter to an entry to limit the characters that it will accept
 * based the contents of the provided #Elm_Entry_Filter_Limit_Size.
 * The function works on the UTF-8 representation of the string, converting
 * it from the set markup, thus not accounting for any format in it.
 *
 * The user must create an #Elm_Entry_Filter_Limit_Size structure and pass
 * it as data when setting the filter. In it, it's possible to set limits
 * by character count or bytes (any of them is disabled if 0), and both can
 * be set at the same time. In that case, it first checks for characters,
 * then bytes. The #Elm_Entry_Filter_Limit_Size structure must be alive and
 * valid for as long as the entry is alive AND the elm_entry_filter_limit_size
 * filter is set.
 *
 * The function will cut the inserted text in order to allow only the first
 * number of characters that are still allowed. The cut is made in
 * characters, even when limiting by bytes, in order to always contain
 * valid ones and avoid half unicode characters making it in.
 *
 * This filter, like any others, does not apply when setting the entry text
 * directly with elm_object_text_set().
 *
 * @ingroup Entry
 */
EAPI void elm_entry_filter_limit_size(void *data, Evas_Object *entry, char **text);

/**
 * @typedef Elm_Entry_Filter_Accept_Set
 *
 * Data for the elm_entry_filter_accept_set() entry filter.
 */
typedef struct _Elm_Entry_Filter_Accept_Set Elm_Entry_Filter_Accept_Set;

/**
 * @struct _Elm_Entry_Filter_Accept_Set
 *
 * Data for the elm_entry_filter_accept_set() entry filter.
 */
struct _Elm_Entry_Filter_Accept_Set
{
   const char *accepted;      /**< Set of characters accepted in the entry. */
   const char *rejected;      /**< Set of characters rejected from the entry. */
};

/**
 * Filter inserted text based on accepted or rejected sets of characters
 *
 * Add this filter to an entry to restrict the set of accepted characters
 * based on the sets in the provided #Elm_Entry_Filter_Accept_Set.
 * This structure contains both accepted and rejected sets, but they are
 * mutually exclusive. This structure must be available for as long as
 * the entry is alive AND the elm_entry_filter_accept_set is being used.
 *
 * The @c accepted set takes preference, so if it is set, the filter will
 * only work based on the accepted characters, ignoring anything in the
 * @c rejected value. If @c accepted is @c NULL, then @c rejected is used.
 *
 * In both cases, the function filters by matching utf8 characters to the
 * raw markup text, so it can be used to remove formatting tags.
 *
 * This filter, like any others, does not apply when setting the entry text
 * directly with elm_object_text_set()
 *
 * @ingroup Entry
 */
EAPI void                   elm_entry_filter_accept_set(void *data, Evas_Object *entry, char **text);

/**
 * Returns the input method context of the entry.
 *
 * This function exposes the internal input method context.
 *
 * IMPORTANT: Many functions may change (i.e delete and create a new one)
 * the internal input method context. Do NOT cache the returned object.
 *
 * @param obj The entry object
 * @return The input method context (Ecore_IMF_Context *) in entry.
 *
 * @ingroup Entry
 */
EAPI void                  *elm_entry_imf_context_get(Evas_Object *obj);

/**
 * @typedef Elm_Cnp_Mode
 * Enum of entry's copy & paste policy.
 *
 * @see elm_entry_cnp_mode_set()
 * @see elm_entry_cnp_mode_get()
 */
typedef enum {
   ELM_CNP_MODE_MARKUP,   /**< copy & paste text with markup tag */
   ELM_CNP_MODE_NO_IMAGE, /**< copy & paste text without item(image) tag */
   ELM_CNP_MODE_PLAINTEXT /**< copy & paste text without markup tag */
} Elm_Cnp_Mode;

/**
 * Control pasting of text and images for the widget.
 *
 * Normally the entry allows both text and images to be pasted.
 * By setting cnp_mode to be #ELM_CNP_MODE_NO_IMAGE, this prevents images from being copy or past.
 * By setting cnp_mode to be #ELM_CNP_MODE_PLAINTEXT, this remove all tags in text .
 *
 * @note this only changes the behaviour of text.
 *
 * @param obj The entry object
 * @param cnp_mode One of #Elm_Cnp_Mode: #ELM_CNP_MODE_MARKUP, #ELM_CNP_MODE_NO_IMAGE, #ELM_CNP_MODE_PLAINTEXT.
 *
 * @ingroup Entry
 */
EAPI void         elm_entry_cnp_mode_set(Evas_Object *obj, Elm_Cnp_Mode cnp_mode);

/**
 * Getting elm_entry text paste/drop mode.
 *
 * Normally the entry allows both text and images to be pasted.
 * This gets the copy & paste mode of the entry.
 *
 * @param obj The entry object
 * @return mode One of #Elm_Cnp_Mode: #ELM_CNP_MODE_MARKUP, #ELM_CNP_MODE_NO_IMAGE, #ELM_CNP_MODE_PLAINTEXT.
 *
 * @ingroup Entry
 */
EAPI Elm_Cnp_Mode elm_entry_cnp_mode_get(const Evas_Object *obj);


#endif

} // end namespace Elmxx

#endif // ELMXX_ENTRY_H
