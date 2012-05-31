# Copyright (c) 2010 ProFUSION embedded systems
#
# This file is part of python-elementary.
#
# python-elementary is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# python-elementary is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with python-elementary.  If not, see <http://www.gnu.org/licenses/>.
#

import traceback

cdef _py_elm_genlist_item_call(func, c_evas.Evas_Object *obj, const_char_ptr part, data) with gil:
    if part != NULL:
        p = part
    else:
        p = None # is it possible?

    try:
        o = evas.c_evas._Object_from_instance(<long>obj)
        return func(o, p, data)
    except Exception, e:
        traceback.print_exc()
        return None

cdef char *_py_elm_genlist_item_text_get(void *data, c_evas.Evas_Object *obj, const_char_ptr part) with gil:
    cdef object prm = <object>data
    cdef GenlistItemClass itc = prm[0]

    func = itc._text_get_func
    if func is None:
        return NULL

    ret = _py_elm_genlist_item_call(func, obj, part, prm[1])
    if ret is not None:
        return strdup(ret)
    else:
        return NULL

cdef c_evas.Evas_Object *_py_elm_genlist_item_content_get(void *data, c_evas.Evas_Object *obj, const_char_ptr part) with gil:
    cdef object prm = <object>data
    cdef c_evas.Object icon
    cdef GenlistItemClass itc = prm[0]

    func = itc._content_get_func
    if func is None:
        return NULL

    ret = _py_elm_genlist_item_call(func, obj, part, prm[1])
    if ret is not None:
        try:
            icon = ret
            return icon.obj
        except Exception, e:
            traceback.print_exc()
            return NULL
    else:
        return NULL

cdef c_evas.Eina_Bool _py_elm_genlist_item_state_get(void *data, c_evas.Evas_Object *obj, const_char_ptr part) with gil:
    cdef object prm = <object>data
    cdef GenlistItemClass itc = prm[0]

    func = itc._state_get_func
    if func is None:
        return False

    ret = _py_elm_genlist_item_call(func, obj, part, prm[1])
    if ret is not None:
        return bool(ret)
    else:
        return False

cdef void _py_elm_genlist_object_item_del(void *data, c_evas.Evas_Object *obj) with gil:
    cdef object prm = <object>data
    cdef GenlistItemClass itc = prm[0]
    cdef GenlistItem item = prm[2]

    func = itc._del_func
    if func is not None:
        try:
            o = evas.c_evas._Object_from_instance(<long>obj)
            func(o, prm[1])
        except Exception, e:
            traceback.print_exc()

    item._unset_obj()

cdef void _py_elm_genlist_item_func(void *data, c_evas.Evas_Object *obj, void *event_info) with gil:
    cdef object prm = <object>data
    cdef object func = prm[3]
    cdef GenlistItem item = prm[2]

    if func is not None:
        try:
            o = evas.c_evas._Object_from_instance(<long>obj)
            func(item, o, prm[1])
        except Exception, e:
            traceback.print_exc()


cdef class GenlistItemClass:
    """Defines the behavior of each list item.

    This class should be created and handled to the Genlist itself.

    It may be subclassed, in this case the methods L{text_get()},
    L{content_get()}, L{state_get()} and L{delete()} will be used.

    It may also be instantiated directly, given getters to override as
    constructor parameters.

    """
    cdef Elm_Genlist_Item_Class obj
    cdef readonly object _item_style
    cdef readonly object _text_get_func
    cdef readonly object _content_get_func
    cdef readonly object _state_get_func
    cdef readonly object _del_func

    def __cinit__(self, *a, **ka):
        self._item_style = "default"
        self._text_get_func = None
        self._content_get_func = None
        self._state_get_func = None
        self._del_func = None

        self.obj.item_style = NULL
        self.obj.func.text_get = _py_elm_genlist_item_text_get
        self.obj.func.content_get = _py_elm_genlist_item_content_get
        self.obj.func.state_get = _py_elm_genlist_item_state_get
        self.obj.func.del_ = _py_elm_genlist_object_item_del

    def __init__(self, item_style=None, text_get_func=None,
                 content_get_func=None, state_get_func=None, del_func=None):
        """GenlistItemClass constructor.

        @parm: B{item_style} the string that defines the genlist item
               theme to be used. The corresponding edje group will
               have this as suffix.

        @parm: B{text_get_func} if provided will override the
               behavior defined by L{text_get()} in this class. Its
               purpose is to return the label string to be used by a
               given part and row. This function should have the
               signature:

                  C{func(obj, part, item_data) -> str}

        @parm: B{content_get_func} if provided will override the behavior
               defined by L{content_get()} in this class. Its purpose is
               to return the icon object to be used (swalloed) by a
               given part and row. This function should have the
               signature:

                  C{func(obj, part, item_data) -> obj}

        @parm: B{state_get_func} if provided will override the
               behavior defined by L{state_get()} in this class. Its
               purpose is to return the boolean state to be used by a
               given part and row. This function should have the
               signature:

                  C{func(obj, part, item_data) -> bool}

        @parm: B{del_func} if provided will override the behavior
               defined by L{del()} in this class. Its purpose is to be
               called when row is deleted, thus finalizing resouces
               and similar. This function should have the signature:

                  C{func(obj, part, item_data) -> str}

        In all these signatures, 'obj' means Genlist and
        'item_data' is the value given to Genlist item append/prepend
        methods, it should represent your row model as you want.
        """
        if item_style:
            self._item_style = str(item_style)

        if text_get_func and not callable(text_get_func):
            raise TypeError("text_get_func is not callable!")
        elif text_get_func:
            self._text_get_func = text_get_func
        else:
            self._text_get_func = self.text_get

        if content_get_func and not callable(content_get_func):
            raise TypeError("content_get_func is not callable!")
        elif content_get_func:
            self._content_get_func = content_get_func
        else:
            self._content_get_func = self.content_get

        if state_get_func and not callable(state_get_func):
            raise TypeError("state_get_func is not callable!")
        elif state_get_func:
            self._state_get_func = state_get_func
        else:
            self._state_get_func = self.state_get

        if del_func and not callable(del_func):
            raise TypeError("del_func is not callable!")
        elif del_func:
            self._del_func = del_func
        else:
            try:
                self._del_func = self.delete
            except AttributeError:
                pass

        self.obj.item_style = self._item_style

    def __str__(self):
        return ("%s(item_style=%r, text_get_func=%s, content_get_func=%s, "
                "state_get_func=%s, del_func=%s)") % \
               (self.__class__.__name__,
                self._item_style,
                self._text_get_func,
                self._content_get_func,
                self._state_get_func,
                self._del_func)

    def __repr__(self):
        return ("%s(%#x, refcount=%d, Elm_Genlist_Item_Class=%#x, "
                "item_style=%r, text_get_func=%s, content_get_func=%s, "
                "state_get_func=%s, del_func=%s)") % \
               (self.__class__.__name__,
                <unsigned long><void *>self,
                PY_REFCOUNT(self),
                <unsigned long>&self.obj,
                self._item_style,
                self._text_get_func,
                self._content_get_func,
                self._state_get_func,
                self._del_func)

    property item_style:
        def __get__(self):
            return self._item_style

    def text_get(self, c_evas.Object obj, char *part, item_data):
        """To be called by Genlist for each row to get its label.

        @parm: B{obj:} the Genlist instance
        @parm: B{part:} the part that is being handled.
        @parm: B{item_data:} the value given to genlist append/prepend.

        @return: label to be used.
        @rtype: str or None
        """
        return None

    def content_get(self, c_evas.Object obj, char *part, item_data):
        """To be called by Genlist for each row to get its icon.

        @parm: B{obj:} the Genlist instance
        @parm: B{part:} the part that is being handled.
        @parm: B{item_data:} the value given to genlist append/prepend.

        @return: icon object to be used and swallowed.
        @rtype: evas Object or None
        """
        return None

    def state_get(self, c_evas.Object obj, char *part, item_data):
        """To be called by Genlist for each row to get its state.

        @parm: B{obj:} the Genlist instance
        @parm: B{part:} the part that is being handled.
        @parm: B{item_data:} the value given to genlist append/prepend.

        @return: boolean state to be used.
        @rtype: bool or None
        """
        return False


cdef class GenlistItem(ObjectItem):
    """
    An item for the genlist widget
    """
    cdef object params

    cdef int _set_obj(self, Elm_Object_Item *obj, params) except 0:
        assert self.obj == NULL, "Object must be clean"
        self.obj = obj
        self.params = params
        Py_INCREF(self)
        return 1

    cdef int _unset_obj(self) except 0:
        assert self.obj != NULL, "Object must wrap something"
        self.obj = NULL
        self.params = None
        Py_DECREF(self)
        return 1

    def __str__(self):
        return "%s(item_class=%s, func=%s, item_data=%s)" % \
               (self.__class__.__name__,
                self.params[0].__class__.__name__,
                self.params[3],
                self.params[1])

    def __repr__(self):
        return ("%s(%#x, refcount=%d, Elm_Object_Item=%#x, "
                "item_class=%s, func=%s, item_data=%r)") % \
               (self.__class__.__name__,
                <unsigned long><void*>self,
                PY_REFCOUNT(self),
                <unsigned long>self.obj,
                self.params[0].__class__.__name__,
                self.params[3],
                self.params[1])

    def data_get(self):
        cdef void* data
        data = elm_object_item_data_get(self.obj)
        if data == NULL:
            return None
        else:
            (item_class, item_data, ret, func) = <object>data
            return item_data

    property data:
        def __get__(self):
            return self.data_get()

    def next_get(self):
        cdef Elm_Object_Item *it
        it = elm_genlist_item_next_get(self.obj)
        return _elm_genlist_item_to_python(it)

    property next:
        def __get__(self):
            return self.next_get()

    def prev_get(self):
        cdef Elm_Object_Item *it
        it = elm_genlist_item_prev_get(self.obj)
        return _elm_genlist_item_to_python(it)

    property prev:
        def __get__(self):
            return self.prev_get()

    def selected_set(self, selected):
        elm_genlist_item_selected_set(self.obj, bool(selected))

    def selected_get(self):
        return bool(elm_genlist_item_selected_get(self.obj))

    property selected:
        def __get__(self):
            return self.selected_get()

        def __set__(self, selected):
            self.selected_set(selected)

    def show(self, scrollto_type = ELM_GENLIST_ITEM_SCROLLTO_IN):
        elm_genlist_item_show(self.obj, scrollto_type)

    def bring_in(self, scrollto_type = ELM_GENLIST_ITEM_SCROLLTO_IN):
        elm_genlist_item_bring_in(self.obj, scrollto_type)

    def update(self):
        elm_genlist_item_update(self.obj)

    #def item_class_update(self, Elm_Genlist_Item_Class itc):
        #elm_genlist_item_item_class_update(self.obj, itc)

    #def item_class_get(self):
        #return elm_genlist_item_item_class_get(self.obj)

    def index_get(self):
        return elm_genlist_item_index_get(self.obj)

    def tooltip_text_set(self, char *text):
        """ Set the text to be shown in the tooltip object

        Setup the text as tooltip object. The object can have only one
        tooltip, so any previous tooltip data is removed.
        Internaly, this method call @tooltip_content_cb_set
        """
        elm_genlist_item_tooltip_text_set(self.obj, text)

    def tooltip_content_cb_set(self, func, *args, **kargs):
        """ Set the content to be shown in the tooltip object

        @param: B{func} Function to be create tooltip content, called when
                need show tooltip.

        Setup the tooltip to object. The object can have only one tooltip,
        so any previews tooltip data is removed. @func(with @{args,kargs}) will
        be called every time that need show the tooltip and it should return a
        valid Evas_Object. This object is then managed fully by tooltip system
        and is deleted when the tooltip is gone.
        """
        if not callable(func):
            raise TypeError("func must be callable")

        cdef void *cbdata

        data = (func, self, args, kargs)
        Py_INCREF(data)
        cbdata = <void *>data
        elm_genlist_item_tooltip_content_cb_set(self.obj,
                                                _tooltip_item_content_create,
                                                cbdata,
                                                _tooltip_item_data_del_cb)

    def item_tooltip_unset(self):
        """ Unset tooltip from object

        Remove tooltip from object. If used the @tool_text_set the internal
        copy of label will be removed correctly. If used
        @tooltip_content_cb_set, the data will be unreferred but no freed.
        """
        elm_genlist_item_tooltip_unset(self.obj)

    def tooltip_style_set(self, style=None):
        """ Sets a different style for this object tooltip.

        @note before you set a style you should define a tooltip with
        elm_genlist_item_tooltip_content_cb_set() or
        elm_genlist_item_tooltip_text_set()
        """
        if style:
            elm_genlist_item_tooltip_style_set(self.obj, style)
        else:
            elm_genlist_item_tooltip_style_set(self.obj, NULL)

    def tooltip_style_get(self):
        """ Get the style for this object tooltip.
        """
        cdef const_char_ptr style
        style = elm_genlist_item_tooltip_style_get(self.obj)
        if style == NULL:
            return None
        return style

    def tooltip_window_mode_set(self, disable):
        return bool(elm_genlist_item_tooltip_window_mode_set(self.obj, disable))

    def tooltip_window_mode_get(self):
        return bool(elm_genlist_item_tooltip_window_mode_get(self.obj))

    def cursor_set(self, char *cursor):
        """ Set the cursor to be shown when mouse is over the genlist item

        Set the cursor that will be displayed when mouse is over the
        item. The item can have only one cursor set to it, so if
        this function is called twice for an item, the previous set
        will be unset.
        """
        elm_genlist_item_cursor_set(self.obj, cursor)

    def cursor_get(self):
        return elm_genlist_item_cursor_get(self.obj)

    def cursor_unset(self):
        """  Unset the cursor to be shown when mouse is over the genlist item
        """
        elm_genlist_item_cursor_unset(self.obj)

    def cursor_style_set(self, style=None):
        """ Sets a different style for this object cursor.

        @note before you set a style you should define a cursor with
        elm_genlist_item_cursor_set()
        """
        if style:
            elm_genlist_item_cursor_style_set(self.obj, style)
        else:
            elm_genlist_item_cursor_style_set(self.obj, NULL)

    def cursor_style_get(self):
        """ Get the style for this object cursor.
        """
        cdef const_char_ptr style
        style = elm_genlist_item_cursor_style_get(self.obj)
        if style == NULL:
            return None
        return style

    def cursor_engine_only_set(self, engine_only):
        """ Sets cursor engine only usage for this object.

        @note before you set engine only usage you should define a cursor with
        elm_genlist_item_cursor_set()
        """
        elm_genlist_item_cursor_engine_only_set(self.obj, bool(engine_only))

    def cursor_engine_only_get(self):
        """ Get the engine only usage for this object.
        """
        return elm_genlist_item_cursor_engine_only_get(self.obj)

    def parent_get(self):
        cdef Elm_Object_Item *it
        it = elm_genlist_item_parent_get(self.obj)
        return _elm_genlist_item_to_python(it)

    property parent:
        def __get__(self):
            return self.parent_get()

    def subitems_clear(self):
        elm_genlist_item_subitems_clear(self.obj)

    def expanded_set(self, expanded):
        elm_genlist_item_expanded_set(self.obj, bool(expanded))

    def expanded_get(self, ):
        return bool(elm_genlist_item_expanded_get(self.obj))

    property expanded:
        def __get__(self):
            return self.expanded_get()

        def __set__(self, expanded):
            self.expanded_set(expanded)

    def expanded_depth_get(self):
        return elm_genlist_item_expanded_depth_get(self.obj)

    def all_contents_unset(self):
        cdef Elm_Object_Item *it
        cdef c_evas.Eina_List *lst

        elm_genlist_item_all_contents_unset(self.obj, &lst)
        ret = []
        ret_append = ret.append
        while lst:
            it = <Elm_Object_Item *>lst.data
            lst = lst.next
            o = _elm_genlist_item_to_python(it)
            if o is not None:
                ret_append(o)
        return ret

    def promote(self):
        elm_genlist_item_promote(self.obj)

    def demote(self):
        elm_genlist_item_demote(self.obj)

    def fields_update(self, parts, itf):
        elm_genlist_item_fields_update(self.obj, parts, itf)

    def decorate_mode_set(self, decorate_it_type, decorate_it_set):
        elm_genlist_item_decorate_mode_set(self.obj, decorate_it_type, decorate_it_set)

    def decorate_mode_get(self):
        return elm_genlist_item_decorate_mode_get(self.obj)

    def type_get(self):
        cdef Elm_Genlist_Item_Type ittype = elm_genlist_item_type_get(self.obj)
        return <Elm_Genlist_Item_Type>ittype

    def flip_set(self, flip):
        elm_genlist_item_flip_set(self.obj, flip)

    def flip_get(self):
        return bool(elm_genlist_item_flip_get(self.obj))

    def select_mode_set(self, mode):
        elm_genlist_item_select_mode_set(self.obj, mode)

    def select_mode_get(self):
        return elm_genlist_item_select_mode_get(self.obj)

def _genlist_item_conv(long addr):
    cdef Elm_Object_Item *it = <Elm_Object_Item *>addr
    cdef void *data = elm_object_item_data_get(it)
    if data == NULL:
        return None
    else:
        prm = <object>data
        return prm[2]

cdef Elm_Object_Item *_elm_genlist_item_from_python(GenlistItem item):
    if item is None:
        return NULL
    else:
        return item.obj

cdef _elm_genlist_item_to_python(Elm_Object_Item *it):
    cdef void *data
    cdef object prm
    if it == NULL:
        return None
    data = elm_object_item_data_get(it)
    if data == NULL:
        return None
    prm = <object>data
    return prm[2]

cdef public class Genlist(Object) [object PyElementaryGenlist, type PyElementaryGenlist_Type]:
    """Creates a generic, scalable and extensible list widget.

    Unlike L{List}, this widget allows more items while keeping
    performance. The items might contain subitems, thus being able to
    do 'tree' hierarchy. The rows may have different look and feel,
    not being restricted only to icon and label.
    """
    def __init__(self, c_evas.Object parent):
        Object.__init__(self, parent.evas)
        self._set_obj(elm_genlist_add(parent.obj))

    def clear(self):
        elm_genlist_clear(self.obj)

    def multi_select_set(self, multi):
        elm_genlist_multi_select_set(self.obj, bool(multi))

    def multi_select_get(self):
        return bool(elm_genlist_multi_select_get(self.obj))

    def mode_set(self, mode = ELM_LIST_COMPRESS):
        elm_genlist_mode_set(self.obj, mode)

    def mode_get(self):
        return elm_genlist_mode_get(self.obj)

    def bounce_set(self, h_bounce, v_bounce):
        elm_genlist_bounce_set(self.obj, bool(h_bounce), bool(v_bounce))

    def bounce_get(self):
        cdef c_evas.Eina_Bool h_bounce, v_bounce
        elm_genlist_bounce_get(self.obj, &h_bounce, &v_bounce)
        return (h_bounce, v_bounce)

    def item_append(self, GenlistItemClass item_class not None, item_data,
                    GenlistItem parent_item=None,
                    int flags=ELM_GENLIST_ITEM_NONE,
                    func=None):
        """Append a new item (add as last row) to this genlist.

        @parm: B{item_class:} a valid instance that defines the
               behavior of this row. See L{GenlistItemClass}.
        @parm: B{item_data:} some data that defines the model of this
               row. This value will be given to methods of
               C{item_class} such as
               L{GenlistItemClass.text_get()}. It will also be
               provided to C{func} as its last parameter.
        @parm: B{parent_item:} if this is a tree child, then the
               parent item must be given here, otherwise it may be
               None. The parent must have the flag
               C{ELM_GENLIST_ITEM_SUBITEMS} set.
        @parm: B{flags:} defines special behavior of this item:
                - ELM_GENLIST_ITEM_NONE = 0
                - ELM_GENLIST_ITEM_SUBITEMS = 1
                - ELM_GENLIST_ITEM_GROUP = 2
        @parm: B{func:} if not None, this must be a callable to be
               called back when the item is selected. The function
               signature is:
                 C{func(item, obj, item_data)}
               Where C{item} is the handle, C{obj} is the Evas object
               that represents this item, and C{item_data} is the
               value given as parameter to this function.
        """
        cdef GenlistItem ret = GenlistItem()
        cdef Elm_Object_Item *item, *parent
        cdef c_evas.Evas_Smart_Cb cb

        parent = _elm_genlist_item_from_python(parent_item)

        if func is None:
            cb = NULL
        elif callable(func):
            cb = _py_elm_genlist_item_func
        else:
            raise TypeError("func is not None or callable")

        # note: keep this positions sync'ed with the rest of the code:
        prm = (item_class, item_data, ret, func)
        item = elm_genlist_item_append(self.obj, &item_class.obj, <void*>prm,
                                       parent,
                                       <Elm_Genlist_Item_Type>flags, cb,
                                       <void*>prm)
        if item != NULL:
            ret._set_obj(item, prm)
            return ret
        else:
            return None

    def item_prepend(self, GenlistItemClass item_class not None, item_data,
                     GenlistItem parent_item=None,
                     int flags=ELM_GENLIST_ITEM_NONE,
                     func=None):
        """Prepend a new item (add as first row) to this genlist.

        @parm: B{item_class:} a valid instance that defines the
               behavior of this row. See L{GenlistItemClass}.
        @parm: B{item_data:} some data that defines the model of this
               row. This value will be given to methods of
               C{item_class} such as
               L{GenlistItemClass.text_get()}. It will also be
               provided to C{func} as its last parameter.
        @parm: B{parent_item:} if this is a tree child, then the
               parent item must be given here, otherwise it may be
               None. The parent must have the flag
               C{ELM_GENLIST_ITEM_SUBITEMS} set.
        @parm: B{flags:} defines special behavior of this item:
                - ELM_GENLIST_ITEM_NONE = 0
                - ELM_GENLIST_ITEM_SUBITEMS = 1
                - ELM_GENLIST_ITEM_GROUP = 2
        @parm: B{func:} if not None, this must be a callable to be
               called back when the item is selected. The function
               signature is:
                 C{func(item, obj, item_data)}
               Where C{item} is the handle, C{obj} is the Evas object
               that represents this item, and C{item_data} is the
               value given as parameter to this function.
        """
        cdef GenlistItem ret = GenlistItem()
        cdef Elm_Object_Item *item, *parent
        cdef c_evas.Evas_Smart_Cb cb

        parent = _elm_genlist_item_from_python(parent_item)

        if func is None:
            cb = NULL
        elif callable(func):
            cb = _py_elm_genlist_item_func
        else:
            raise TypeError("func is not None or callable")

        # note: keep this positions sync'ed with the rest of the code:
        prm = (item_class, item_data, ret, func)
        item = elm_genlist_item_prepend(self.obj, &item_class.obj, <void*>prm,
                                        parent,
                                        <Elm_Genlist_Item_Type>flags, cb,
                                        <void*>prm)
        if item != NULL:
            ret._set_obj(item, prm)
            return ret
        else:
            return None

    def item_insert_before(self, GenlistItemClass item_class not None,
                           item_data, GenlistItem before_item=None,
                           int flags=ELM_GENLIST_ITEM_NONE,
                           func=None):
        """Insert a new item (row) before another item in this genlist.

        @parm: B{item_class:} a valid instance that defines the
               behavior of this row. See L{GenlistItemClass}.
        @parm: B{item_data:} some data that defines the model of this
               row. This value will be given to methods of
               C{item_class} such as
               L{GenlistItemClass.text_get()}. It will also be
               provided to C{func} as its last parameter.
        @parm: B{before_item:} a reference item to use, the new item
               will be inserted before it.
        @parm: B{flags:} defines special behavior of this item:
                - ELM_GENLIST_ITEM_NONE = 0
                - ELM_GENLIST_ITEM_SUBITEMS = 1
                - ELM_GENLIST_ITEM_GROUP = 2
        @parm: B{func:} if not None, this must be a callable to be
               called back when the item is selected. The function
               signature is:
                 C{func(item, obj, item_data)}
               Where C{item} is the handle, C{obj} is the Evas object
               that represents this item, and C{item_data} is the
               value given as parameter to this function.
        """
        cdef GenlistItem ret = GenlistItem()
        cdef Elm_Object_Item *item, *before
        cdef c_evas.Evas_Smart_Cb cb

        before = _elm_genlist_item_from_python(before_item)

        if func is None:
            cb = NULL
        elif callable(func):
            cb = _py_elm_genlist_item_func
        else:
            raise TypeError("func is not None or callable")

        # note: keep this positions sync'ed with the rest of the code:
        prm = (item_class, item_data, ret, func)
        item = elm_genlist_item_insert_before(self.obj, &item_class.obj,
                                              <void*>prm,
                                              NULL,
                                              before,
                                              <Elm_Genlist_Item_Type>flags, cb,
                                              <void*>prm)
        if item != NULL:
            ret._set_obj(item, prm)
            return ret
        else:
            return None

    def item_insert_after(self, GenlistItemClass item_class not None,
                          item_data, GenlistItem after_item=None,
                          int flags=ELM_GENLIST_ITEM_NONE,
                          func=None):
        """Insert a new item (row) after another item in this genlist.

        @parm: B{item_class:} a valid instance that defines the
               behavior of this row. See L{GenlistItemClass}.
        @parm: B{item_data:} some data that defines the model of this
               row. This value will be given to methods of
               C{item_class} such as
               L{GenlistItemClass.text_get()}. It will also be
               provided to C{func} as its last parameter.
        @parm: B{after_item:} a reference item to use, the new item
               will be inserted after it.
        @parm: B{flags:} defines special behavior of this item:
                - ELM_GENLIST_ITEM_NONE = 0
                - ELM_GENLIST_ITEM_SUBITEMS = 1
                - ELM_GENLIST_ITEM_GROUP = 2
        @parm: B{func:} if not None, this must be a callable to be
               called back when the item is selected. The function
               signature is:
                 C{func(item, obj, item_data)}
               Where C{item} is the handle, C{obj} is the Evas object
               that represents this item, and C{item_data} is the
               value given as parameter to this function.
        """
        cdef GenlistItem ret = GenlistItem()
        cdef Elm_Object_Item *item, *after
        cdef c_evas.Evas_Smart_Cb cb

        after = _elm_genlist_item_from_python(after_item)

        if func is None:
            cb = NULL
        elif callable(func):
            cb = _py_elm_genlist_item_func
        else:
            raise TypeError("func is not None or callable")

        # note: keep this positions sync'ed with the rest of the code:
        prm = (item_class, item_data, ret, func)
        item = elm_genlist_item_insert_after(self.obj, &item_class.obj,
                                             <void*>prm,
                                             NULL,
                                             after,
                                             <Elm_Genlist_Item_Type>flags, cb,
                                             <void*>prm)
        if item != NULL:
            ret._set_obj(item, prm)
            return ret
        else:
            return None

    #Elm_Object_Item         *elm_genlist_item_sorted_insert(self.obj, Elm_Genlist_Item_Class *itc, void *data, Elm_Object_Item *parent, Elm_Genlist_Item_Type flags, evas.c_evas.Eina_Compare_Cb comp, evas.c_evas.Evas_Smart_Cb func, void *func_data)

    def selected_item_get(self):
        cdef Elm_Object_Item *it
        it = elm_genlist_selected_item_get(self.obj)
        return _elm_genlist_item_to_python(it)

    property selected_item:
        def __get__(self):
            return self.selected_item_get()

    def selected_items_get(self):
        cdef Elm_Object_Item *it
        cdef c_evas.const_Eina_List *lst

        lst = elm_genlist_selected_items_get(self.obj)
        ret = []
        ret_append = ret.append
        while lst:
            it = <Elm_Object_Item *>lst.data
            lst = lst.next
            o = _elm_genlist_item_to_python(it)
            if o is not None:
                ret_append(o)
        return ret

    def realized_items_get(self):
        cdef Elm_Object_Item *it
        cdef c_evas.Eina_List *lst

        lst = elm_genlist_realized_items_get(self.obj)
        ret = []
        ret_append = ret.append
        while lst:
            it = <Elm_Object_Item *>lst.data
            lst = c_evas.eina_list_remove_list(lst, lst)
            o = _elm_genlist_item_to_python(it)
            if o is not None:
                ret_append(o)
        return ret

    def first_item_get(self):
        cdef Elm_Object_Item *it
        it = elm_genlist_first_item_get(self.obj)
        return _elm_genlist_item_to_python(it)

    property first_item:
        def __get__(self):
            return self.first_item_get()

    def last_item_get(self):
        cdef Elm_Object_Item *it
        it = elm_genlist_last_item_get(self.obj)
        return _elm_genlist_item_to_python(it)

    property last_item:
        def __get__(self):
            return self.last_item_get()

    def scroller_policy_set(self, policy_h, policy_v):
        elm_genlist_scroller_policy_set(self.obj, policy_h, policy_v)

    def scroller_policy_get(self):
        cdef Elm_Scroller_Policy policy_h, policy_v
        elm_genlist_scroller_policy_get(self.obj, &policy_h, &policy_v)
        return (policy_h, policy_v)

    def realized_items_update(self):
        elm_genlist_realized_items_update(self.obj)

    def items_count(self):
        return elm_genlist_items_count(self.obj)

    def homogeneous_set(self, homogeneous):
        elm_genlist_homogeneous_set(self.obj, bool(homogeneous))

    def homogeneous_get(self):
        return bool(elm_genlist_homogeneous_get(self.obj))

    def block_count_set(self, int n):
        elm_genlist_block_count_set(self.obj, n)

    def block_count_get(self):
        return elm_genlist_block_count_get(self.obj)

    def longpress_timeout_set(self, timeout):
        elm_genlist_longpress_timeout_set(self.obj, timeout)

    def longpress_timeout_get(self):
        return elm_genlist_longpress_timeout_get(self.obj)

    def at_xy_item_get(self, int x, int y):
        cdef Elm_Object_Item *it
        it = elm_genlist_at_xy_item_get(self.obj, x, y, NULL)
        return _elm_genlist_item_to_python(it)

    def decorated_item_get(self):
        cdef Elm_Object_Item *it
        it = elm_genlist_decorated_item_get(self.obj)
        return _elm_genlist_item_to_python(it)

    def reorder_mode_set(self, reorder_mode):
        elm_genlist_reorder_mode_set(self.obj, reorder_mode)

    def reorder_mode_get(self):
        return bool(elm_genlist_reorder_mode_get(self.obj))

    def decorate_mode_set(self, decorated):
        elm_genlist_decorate_mode_set(self.obj, decorated)

    def decorate_mode_get(self):
        return bool(elm_genlist_decorate_mode_get(self.obj))

    def tree_effect_enabled_set(self, enabled):
        elm_genlist_tree_effect_enabled_set(self.obj, enabled)

    def tree_effect_enabled_get(self):
        return bool(elm_genlist_tree_effect_enabled_get(self.obj))

    def highlight_mode_set(self, highlight):
        elm_genlist_highlight_mode_set(self.obj, highlight)

    def highlight_mode_get(self):
        return bool(elm_genlist_highlight_mode_get(self.obj))

    def select_mode_set(self, mode):
        elm_genlist_select_mode_set(self.obj, mode)

    def select_mode_get(self):
        return elm_genlist_select_mode_get(self.obj)

    def callback_activated_add(self, func, *args, **kwargs):
        self._callback_add_full("activated", _genlist_item_conv,
                                func, *args, **kwargs)

    def callback_activated_del(self, func):
        self._callback_del_full("activated",  _genlist_item_conv, func)

    def callback_clicked_double_add(self, func, *args, **kwargs):
        self._callback_add_full("clicked,double", _genlist_item_conv,
                                func, *args, **kwargs)

    def callback_clicked_double_del(self, func):
        self._callback_del_full("clicked,double",  _genlist_item_conv, func)

    def callback_selected_add(self, func, *args, **kwargs):
        self._callback_add_full("selected", _genlist_item_conv,
                                func, *args, **kwargs)

    def callback_selected_del(self, func):
        self._callback_del_full("selected", _genlist_item_conv, func)

    def callback_unselected_add(self, func, *args, **kwargs):
        self._callback_add_full("unselected", _genlist_item_conv,
                                func, *args, **kwargs)

    def callback_unselected_del(self, func):
        self._callback_del_full("unselected",  _genlist_item_conv, func)

    def callback_expanded_add(self, func, *args, **kwargs):
        self._callback_add_full("expanded", _genlist_item_conv,
                                func, *args, **kwargs)

    def callback_expanded_del(self, func):
        self._callback_del_full("expanded",  _genlist_item_conv, func)

    def callback_contracted_add(self, func, *args, **kwargs):
        self._callback_add_full("contracted", _genlist_item_conv,
                                func, *args, **kwargs)

    def callback_contracted_del(self, func):
        self._callback_del_full("contracted",  _genlist_item_conv, func)

    def callback_expand_request_add(self, func, *args, **kwargs):
        self._callback_add_full("expand,request", _genlist_item_conv,
                                func, *args, **kwargs)

    def callback_expand_request_del(self, func):
        self._callback_del_full("expand,request",  _genlist_item_conv, func)

    def callback_contract_request_add(self, func, *args, **kwargs):
        self._callback_add_full("contract,request", _genlist_item_conv,
                                func, *args, **kwargs)

    def callback_contract_request_del(self, func):
        self._callback_del_full("contract,request",  _genlist_item_conv, func)

    def callback_realized_add(self, func, *args, **kwargs):
        self._callback_add_full("realized", _genlist_item_conv,
                                func, *args, **kwargs)

    def callback_realized_del(self, func):
        self._callback_del_full("realized",  _genlist_item_conv, func)

    def callback_unrealized_add(self, func, *args, **kwargs):
        self._callback_add_full("unrealized", _genlist_item_conv,
                                func, *args, **kwargs)

    def callback_unrealized_del(self, func):
        self._callback_del_full("unrealized",  _genlist_item_conv, func)

    def callback_drag_start_up_add(self, func, *args, **kwargs):
        self._callback_add_full("drag,start,up", _genlist_item_conv,
                                func, *args, **kwargs)

    def callback_drag_start_up_del(self, func):
        self._callback_del_full("drag,start,up",  _genlist_item_conv, func)

    def callback_drag_start_down_add(self, func, *args, **kwargs):
        self._callback_add_full("drag,start,down", _genlist_item_conv,
                                func, *args, **kwargs)

    def callback_drag_start_down_del(self, func):
        self._callback_del_full("drag,start,down",  _genlist_item_conv, func)

    def callback_drag_start_left_add(self, func, *args, **kwargs):
        self._callback_add_full("drag,start,left", _genlist_item_conv,
                                func, *args, **kwargs)

    def callback_drag_start_left_del(self, func):
        self._callback_del_full("drag,start,left",  _genlist_item_conv, func)

    def callback_drag_start_right_add(self, func, *args, **kwargs):
        self._callback_add_full("drag,start,right", _genlist_item_conv,
                                func, *args, **kwargs)

    def callback_drag_start_right_del(self, func):
        self._callback_del_full("drag,start,right",  _genlist_item_conv, func)

    def callback_drag_stop_add(self, func, *args, **kwargs):
        self._callback_add_full("drag,stop", _genlist_item_conv,
                                func, *args, **kwargs)

    def callback_drag_stop_del(self, func):
        self._callback_del_full("drag,stop",  _genlist_item_conv, func)

    def callback_drag_add(self, func, *args, **kwargs):
        self._callback_add_full("drag", _genlist_item_conv,
                                func, *args, **kwargs)

    def callback_drag_del(self, func):
        self._callback_del_full("drag",  _genlist_item_conv, func)

    def callback_longpressed_add(self, func, *args, **kwargs):
        self._callback_add_full("longpressed", _genlist_item_conv,
                                func, *args, **kwargs)

    def callback_longpressed_del(self, func):
        self._callback_del_full("longpressed", _genlist_item_conv, func)

    def callback_scroll_anim_start_add(self, func, *args, **kwargs):
        self._callback_add("scroll,anim,start", func, *args, **kwargs)

    def callback_scroll_anim_start_del(self, func):
        self._callback_del("scroll,anim,start", func)

    def callback_scroll_anim_stop_add(self, func, *args, **kwargs):
        self._callback_add("scroll,anim,stop", func, *args, **kwargs)

    def callback_scroll_anim_stop_del(self, func):
        self._callback_del("scroll,anim,stop", func)

    def callback_scroll_drag_start_add(self, func, *args, **kwargs):
        self._callback_add("scroll,drag,start", func, *args, **kwargs)

    def callback_scroll_drag_start_del(self, func):
        self._callback_del("scroll,drag,start", func)

    def callback_scroll_drag_stop_add(self, func, *args, **kwargs):
        self._callback_add("scroll,drag,stop", func, *args, **kwargs)

    def callback_scroll_drag_stop_del(self, func):
        self._callback_del("scroll,drag,stop", func)

    def callback_edge_top_add(self, func, *args, **kwargs):
        self._callback_add("edge,top", func, *args, **kwargs)

    def callback_edge_top_del(self, func):
        self._callback_del("edge,top", func)

    def callback_edge_bottom_add(self, func, *args, **kwargs):
        self._callback_add("edge,bottom", func, *args, **kwargs)

    def callback_edge_bottom_del(self, func):
        self._callback_del("edge,bottom", func)

    def callback_edge_left_add(self, func, *args, **kwargs):
        self._callback_add("edge,left", func, *args, **kwargs)

    def callback_edge_left_del(self, func):
        self._callback_del("edge,left", func)

    def callback_edge_right_add(self, func, *args, **kwargs):
        self._callback_add("edge,right", func, *args, **kwargs)

    def callback_edge_right_del(self, func):
        self._callback_del("edge,right", func)

    def callback_multi_swipe_left_add(self, func, *args, **kwargs):
        self._callback_add("multi,swipe,left", func, *args, **kwargs)

    def callback_multi_swipe_left_del(self, func):
        self._callback_del("multi,swipe,left", func)

    def callback_multi_swipe_right_add(self, func, *args, **kwargs):
        self._callback_add("multi,swipe,right", func, *args, **kwargs)

    def callback_multi_swipe_right_del(self, func):
        self._callback_del("multi,swipe,right", func)

    def callback_multi_swipe_up_add(self, func, *args, **kwargs):
        self._callback_add("multi,swipe,up", func, *args, **kwargs)

    def callback_multi_swipe_up_del(self, func):
        self._callback_del("multi,swipe,up", func)

    def callback_multi_swipe_down_add(self, func, *args, **kwargs):
        self._callback_add("multi,swipe,down", func, *args, **kwargs)

    def callback_multi_swipe_down_del(self, func):
        self._callback_del("multi,swipe,down", func)

    def callback_multi_pinch_out_add(self, func, *args, **kwargs):
        self._callback_add("multi,pinch,out", func, *args, **kwargs)

    def callback_multi_pinch_out_del(self, func):
        self._callback_del("multi,pinch,out", func)

    def callback_multi_pinch_in_add(self, func, *args, **kwargs):
        self._callback_add("multi,pinch,in", func, *args, **kwargs)

    def callback_multi_pinch_in_del(self, func):
        self._callback_del("multi,pinch,in", func)

    def callback_swipe_add(self, func, *args, **kwargs):
        self._callback_add("swipe", func, *args, **kwargs)

    def callback_swipe_del(self, func):
        self._callback_del("swipe", func)

    def callback_moved_add(self, func, *args, **kwargs):
        self._callback_add_full("moved", _genlist_item_conv,
                                func, *args, **kwargs)

    def callback_moved_del(self, func):
        self._callback_del_full("moved",  _genlist_item_conv, func)

    def callback_moved_after_add(self, func, *args, **kwargs):
        self._callback_add_full("moved,after", _genlist_item_conv,
                                func, *args, **kwargs)

    def callback_moved_after_del(self, func):
        self._callback_del_full("moved,after",  _genlist_item_conv, func)

    def callback_moved_before_add(self, func, *args, **kwargs):
        self._callback_add_full("moved,before", _genlist_item_conv,
                                func, *args, **kwargs)

    def callback_moved_before_del(self, func):
        self._callback_del_full("moved,before",  _genlist_item_conv, func)

    def callback_language_changed_add(self, func, *args, **kwargs):
        self._callback_add("language,changed", func, *args, **kwargs)

    def callback_language_changed_del(self, func):
        self._callback_del("language,changed", func)

    def callback_tree_effect_finished_add(self, func, *args, **kwargs):
        self._callback_add("tree,effect,finished", func, *args, **kwargs)

    def callback_tree_effect_finished_del(self, func):
        self._callback_del("tree,effect,finished", func)

_elm_widget_type_register("genlist", Genlist)

cdef extern from "Elementary.h": # hack to force type to be known
    cdef PyTypeObject PyElementaryGenlist_Type # hack to install metaclass
_install_metaclass(&PyElementaryGenlist_Type, ElementaryObjectMeta)
