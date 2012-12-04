Class = require('class').Class;
Property = require('class').Property;
elm = require('elm.so');
fs = require('fs');
taffy = require('taffy').taffy;

themes = {
  'default': {
    'app': {
      name: 'themes/eui.edj',
      group: 'app'
    },
    'list': {
      name: 'themes/eui.edj',
      group: 'list'
    },
    'split': {
      name: 'themes/eui.edj',
      group: 'split'
    }
  }
};

defaults = {
  title: 'EasyUI Application',
  themes: themes,
  theme: 'default'
};

/**
 * The base class of controllers.
 * @extends Class
 * @abstract
 */
Controller = Class.extend({

  /**
   * Returns the value of feature, or defaultValue if it's undefined.
   * If feature is a function, returns its return value.
   *
   * @param {Mixed} feature
   * @param {Mixed} defaultValue
   * @protected
   */
  _feature: function(feature, defaultValue) {
    if (feature === undefined)
      return defaultValue;
    if (typeof(feature) === 'function')
      return feature.apply(this);
    return feature;
  },

  /**
   * Low level (unrealized elm object) description of this controller's user interface.
   * @property {Descriptor}
   * @abstract
   */
  viewDescriptor: undefined,

  /**
   * Sets the reference to the realized view descriptor.
   * @param {Realized} view Realized view description
   * @protected
   */
  _setViewContent: function(view) {
    this.viewContent = view.content;
    this.naviframe_item = view;
  },

  /**
   * Realized content of view #viewDescriptor.
   * @type {Realized}
   * @readonly
   */
  view: new Property({
    get: function() { return this.viewContent.content.view; }
  }),

  /**
   * Returns the contents of the #viewDescriptor
   * wrapped in a layout with a toolbar.
   * @return {Descriptor}
   * @protected
   */
  _getViewDescriptor: function() {
    if (this.cachedViewDescriptor !== undefined)
      return this.cachedViewDescriptor;

    this.cachedViewDescriptor = elm.Layout({
      file: EUI.defaults._theme['app'],
      content: {
        view: this.viewDescriptor,
        toolbar: elm.Toolbar({
          select_mode: 'none',
          elements: {}
        })
      }
    });

    return this.cachedViewDescriptor;
  },

  /** Pop the controller that is on top of the stack. */
  popController: function() {
    this.naviframe.pop();
  },

  /**
   * Fired after the controller is popped.
   * @private
   */
  _shutdown: function() {
    if (EUI.__shutting_down)
      return;

    if (this.model)
      this.model.removeListener(this);

    if (this.parent)
      this.parent.evaluateViewChanges();

    if (this.didPopController)
      this.didPopController();
  },

  /**
   * Pushes a {@link Controller} on top of the application stack.
   * @param {Controller} ctrl
   */
  pushController: function(ctrl) {
    ctrl.parent = this;

    if (ctrl instanceof ActionSheet)
      return;

    if (this.naviframe == undefined)
      return;

    if (ctrl.naviframe == undefined)
      ctrl.naviframe = this.naviframe;

    var cnt = Object.keys(ctrl.naviframe.elements).length;
    ctrl.naviframe.elements[cnt] = {
      content: ctrl._getViewDescriptor(),
      style: ctrl.navigationBarStyle,
      on_delete: ctrl._shutdown.bind(ctrl),
    };

    ctrl._setViewContent(ctrl.naviframe.elements[cnt]);
    ctrl.didRealizeView();
  },

  /**
   * Wraps this controller's {@link #viewDescriptor} in an elm.Naviframe, so
   * that controllers can be pushed to or popped from the application stack.
   *
   * It consist in a frame with the result of
   * this._getViewDescriptor() call inside.
   *
   * @return {Descriptor}
   * @protected
   */
  _getWrappedViewDescriptor: function() {
    return elm.Naviframe({
      expand: 'both',
      fill: 'both',
      resize: true,
      title_visible: false,
      elements: [
        {
          'content': this._getViewDescriptor(),
          'on_delete': this._shutdown.bind(this),
          'style': this.navigationBarStyle
        }
      ]
    });
  },

  /**
   * Sets the reference to the realized application view descriptor.
   * @param {Realized} realized
   * @protected
   */
  _setRealizedApp: function(realized) {
    this.naviframe = realized;
    this._setViewContent(realized.elements[0]);
    this.didRealizeView();
  },

  /**
   * Returns true if the #viewDescriptor was realized.
   * @type {Boolean}
   * @protected
   */
  _isRealized: new Property({
    get: function() { return this.naviframe instanceof elm.Naviframe; }
  }),

  /**
   * Returns true if realized is the application itself.
   * @param {Controller} realized
   * @return {Boolean}
   * @protected
   */
  _isEqual: function(realized) {
      return ((this.naviframe) && (realized === this.naviframe));
  },

  /**
   * Fired after the model is changed.
   * @param {Number/Array} indexes
   * @param {String} hint
   * @event
   */
  didChangeModel: function (indexes, hint) {
    this._cachedItem = undefined;
    this.updateView(indexes, hint);
    this.evaluateViewChanges();
  },

  /**
   * Fired before #init is called.
   *
   * Parameters passed to the constructor are passed to this method.
   * @template
   * @event
   */
  willInitialize: function () {},

  /**
   * Triggered when an object of {@link Controller} is instantiated.
   *
   * Abstract initializer method. Parameters passed to the constructor
   * are passed to this method.
   *
   * @template
   * @event
   */
  init: function() {},

  /**
   * Fired after #init was called.
   *
   * Parameters passed to the constructor are passed to this method.
   * @template
   * @event
   */
  didInitialize: function () {},

  /**
   * Fired when an View update is required.
   * @event
   * @template
   * @param {Number/Array} indexes Indexes to be updated
   * @param {String} hint
   */
  updateView: function(indexes, hint) {},

  /**
   * Fired after view is realized.
   * @event
   */
  didRealizeView: function () {
    if (this.model)
      this.model.addListener(this);
    this.updateView();
    this.evaluateViewChanges();
  },

  /**
   * Updates the toolbar content if the #toolbarItems is changed.
   * @private
   */
  _evaluateToolbarChanges: function() {

    if (!this._isRealized) return;

    var items = this.toolbarItems;
    var toolbar = this.viewContent.content.toolbar;

    if (toolbar.cachedItems == items)
      return;

    this.viewContent.signal_emit('bottom,toolbar', '');
    var toolbarVisible = this.toolbarItems.length;
    this.viewContent.signal_emit(toolbarVisible ? 'show,toolbar' : 'hide,toolbar', '');

    for (var i in items) {
      if (!items.hasOwnProperty(i))
        continue;

      if (typeof(items[i]) == 'object') {
        items[i].on_select = this.selectedToolbarItem.bind(this, items[i]);
        toolbar.elements[i] = items[i];
      } else {
        toolbar.elements[i] = {
          label: items[i],
          on_select: this.selectedToolbarItem.bind(this, items[i])
        };
      }
    }

    if (this.didUpdateToolbar)
      this.didUpdateToolbar(toolbar, items);

    toolbar.cachedItems = items;
  },

  /**
   * Creates and returns a Description according to the item type.
   *
   * * If the item is the string 'back' or the item is a reference to its parent controller,
   * then a 'Back' button that pop the current controller will be returned.
   * * If the item is the string 'sidePanel' or the item is a reference to the side panel,
   * then an 'Application' button that show/hide the side panel will be returned.
   * * Otherwise a regular button will be returned. When clicked, this button will
   * fires the #selectedNavigationBarItem event.
   *
   * @param {Mixed} item
   * @return {Descriptor}
   * @private
   */
  _createNavigationBarItem: function(item) {

    if (typeof(item) === 'string') {
      switch (item) {
        case 'back':
          item = this.parent;
          break;
        case 'sidePanel':
          item = this.split.left;
          break;
        default:
          return elm.Button({
            label: item,
            on_click: this.selectedNavigationBarItem.bind(this, item)
          });
      }
    }

    if (item instanceof Controller) {

      var btn = elm.Button({});

      if (item.title)
        btn.label = item.title;

      if (item.icon)
        btn.icon = elm.Icon({ image: item.icon });

      switch (item) {
        case this.parent:
          btn.on_click = this.popController.bind(this);
          break;
        case this.split.left:
          if (EUI.uiMode === 'tablet')
            return;
          btn.icon = btn.icon || elm.Icon({ image: 'Apps' });
          btn.on_click = function() { this.split.leftPanelVisible = true }.bind(this);
          break;
        default:
          btn.on_click = this.selectedNavigationBarItem.bind(this, item);
      }

      return btn;
    }

    if (typeof(item) === 'object' && item.hasOwnProperty('label')) {
      item.on_click = item.on_click || this.selectedNavigationBarItem.bind(this, item);
      return elm.Button(item);
    }

    return item;
  },

  /**
   * Creates and returns a Description of navigation bar items according to items.
   *
   * * If items is an array of items, returns theirs Descriptors in a elm.Box().
   * * Otherwise, returns the items Descriptor.
   *
   * @param {Mixed} items
   * @return {Descriptor}
   * @private
   */
  _createNavigationBarItems: function(items) {
    if (items === undefined || items.length === 0)
      return;
    if (items.length == 1)
      return this._createNavigationBarItem(items[0]);

    var elements = [];
    for (var i = 0, j = items.length; i < j; ++i)
      elements.push(this._createNavigationBarItem(items[i]));

    return elm.Box({ horizontal: true, elements: elements });
  },

  /**
   * Updates the navigation bar if #title or #navigationBarItems are changed.
   * @private
   * @event
   */
  _evaluateNavigationBarChanges: function() {

    if (!this._isRealized) return;

    var items = this.navigationBarItems;
    var title = this.title;

    if (items == this.naviframe.cachedItems && title == this.naviframe.cachedTitle)
      return;

    this.naviframe_item.title = title || '';

    if (items === undefined) {
      this.naviframe.cachedItems = items;
      return;
    }

    var left = [].concat(this._feature(items.left, []));

    if (left.length)
      this.naviframe_item.prev_btn = this._createNavigationBarItems(left);

    var right = [].concat(this._feature(items.right, []));
    this.naviframe_item.next_btn = this._createNavigationBarItems(right);

    if (this.didUpdateNavigationBar)
      this.didUpdateNavigationBar(items);

    this.naviframe.cachedItems = items;
    this.naviframe.cachedTitle = title;
  },

  /**
   * Updates the visibility of view's elements.
   * @private
   * @event
   */
  _updateInterfaceElementVisibility: function() {
    this.naviframe.title_visible =
      (this.title || Object.keys(this.navigationBarItems).length);
  },

  /**
   * Updates the window properties.
   * @private
   * @event
   */
  _updateWindowProperties: function() {
    if (!this._isRealized) return;
    EUI.window.fullscreen = this.isFullscreen;
  },

  /**
   * Updates the view elements if needed.
   * @private
   */
  evaluateViewChanges: function() {
    this._evaluateToolbarChanges();
    this._evaluateNavigationBarChanges();
    this._updateInterfaceElementVisibility();
    this._updateWindowProperties();
  },

  /**
   * Fired when an navigation bar item is clicked.
   * @require #navigationBarItems
   * @template
   * @event
   */
  selectedNavigationBarItem: function(item) {},

  /**
   * Text to be shown on navigation bar
   * and buttons that references this controller.
   * @type {String/Function}
   */
  title: new Property({
    watch: function() { this._evaluateNavigationBarChanges() }
  }),

  /**
   * @type {Object/Function}
   * @require #selectedToolbarItem
   *
   * Items of navigation bar grouped in an Object.
   *
   * The position of the items on the controller title bar is given by the left
   * and right positioning attributes of the navigationBarItems object.
   * Positioning attributes accepts as values single items or an array of items.
   * Items could be strings or objects. On object items, *icon* and *label*
   * attributes are used on items. On string items, the string is used
   * as text of the item.
   *
   * ** Special Cases **
   *
   * * Using the special string 'back' or the parent controller as item, a special
   * button that pops the current controller will be placed on navigation bar.
   * * Using the special string 'sidePanel' or the side panel as item, a special
   * button that shows/hides the side panel will be placed on navigation bar.
   *
   * In regular cases, the #selectedToolbarItem event will be fired when an
   * item is clicked.
   *
   *
   *     this.navigationBarItems = { right: 'next', left: 'previous' };
   *     // or
   *     this.navigationBarItems = function() {
   *         return { right: ['Save', 'New'], left: 'Cancel' };
   *     };
   *
   */
  navigationBarItems: new Property({
    value: {},
    watch: function() { this._evaluateNavigationBarChanges() }
  }),

  /**
   * String that represents the navigation bar style.
   * @type {String/Function}
   */
  navigationBarStyle: new Property({
    watch: function() { this._evaluateNavigationBarChanges() }
  }),

  /**
   * Fired when an item is selected on the controller toolbar.
   * @param {Mixed} item
   * @require #toolbarItems
   * @template
   * @event
   */
  selectedToolbarItem: function(item) {},

  /**
   * Array of toolbar items.
   *
   * Items could be simple strings or objects. On object items, *icon* and
   * *label* attributes will be used as icon and text of the toolbar item. On
   * string items, the string will be used as text of the toolbar item.
   *
   * @require #selectedToolbarItem
   * @type {Array/Function}
   */
  toolbarItems: new Property({
    value: [],
    watch: function() { this._evaluateToolbarChanges() }
  }),

  /**
   * Sets or returns the fullscreen state of the application.
   * @type {Boolean}
   */
  isFullscreen: new Property({
    watch: function() { this._updateWindowProperties() }
  }),

  /**
   * Returns the first {@link SplitController} on the parent
   * chain, or undefined if not found.
   * @type {SplitController}
   * @readonly
   */
  split: new Property({
    get: function() {
      if (this instanceof SplitController)
        return this;
      if (this.parent)
        return this.parent.split;
      return undefined;
    }
  })

});

/**
 * Model base class
 * @extends Class
 * @abstract
 */
Model = Class.extend({

  /**
   * Fired before #init is called.
   * @protected
   * @event
   */
  willInitialize: function(){

    /**
     * Keeps the list of registered listeners.
     * @type {Array}
     * @private
     */
    this.listeners = [];
  },

  /**
   * Abstract initializer method.
   *
   * Parameters passed to the constructor are passed to this method.
   * @template
   * @event
   */
  init: function() {},

  /**
   * Returns the element at index or null if not found.
   * @param {Number} index
   * @return {Mixed}
   */
  itemAtIndex: function(index) {
    return null;
  },

  /**
   * Number of elements in the model.
   * @type {Number}
   * @readonly
   */
  length: new Property({
    value: 0
  }),

  /**
   * Notifies all listeners of change.
   * See #addListener
   * @param {Number/Array} indexes
   * @param {String} hint
   */
  notifyListeners: function(indexes, hint) {
    if (!this.listeners.length) return;

    for (var i = 0; i < this.listeners.length; i++) {
      this.listeners[i].didChangeModel(indexes, hint, this);
    }
  },

  /**
   * Connect listener to this model so it
   * receives events whenever the model changes.
   *
   * A listener must have a didChangeModel method, that
   * will be called on a change in the model. This method
   * has the following signature:
   *
   *     didChangeModel(indexes, hint, model)
   *
   * Where *indexes* is an array with the modified items
   * indexes, *hint* is a string containing an arbitrary
   * hint for the listener and *model* is the
   * changed model.
   * @param {Object} listener
   */
  addListener: function(listener) {
    if (this.listeners.indexOf(listener) < 0)
      this.listeners.push(listener);
  },

  /**
   * Unconnect listener from this model, so it does
   * not receive events whenever this model changes.
   * @param {Object} listener
   */
  removeListener: function(listener){
    var index = this.listeners.indexOf(listener);
    if (index > -1)
      this.listeners.splice(index, 1);
  },

  /**
   * Delete item at specified index.
   * @param {Number} index
   * @template
   */
  deleteItemAtIndex: function(index) {},

  /**
   * Replaces the data of the item at specified index.
   * @param {Number} index Index of element to be replaced.
   * @param {Mixed} data Content to replace with
   * @template
   */
  updateItemAtIndex: function(index, data) {},

  /**
   * Selected item index.
   * @type {Number}
   */
  selectedIndex: new Property({
    get: function() { return this.__selectedIndex; },
    set: function(value) {
      if (typeof(value) !== 'number' || this._selectedIndex === value)
        return;
      this.__selectedIndex = value;
      this.notifyListeners(value, 'select');
    }
  })
});

/** @extend Model */
FilterModel = Model.extend({
  /**
   * @param {Model} proxy_model
   * @param {function} filter_fn
   */
  init: function(proxy_model, filter_fn) {
    this.proxy_model = proxy_model;
    this.filter_fn = filter_fn;
    this.proxy_model.addListener(this);
  },
  /** @inheritdoc */
  itemAtIndex: function(index) {
    var virtualindex = 0;
    for (var i = 0, j = this.proxy_model.length; i < j; i++) {
      var item = this.proxy_model.itemAtIndex(i);
      if (this.filter_fn(item)) {
        if (virtualindex === index)
          return item;
        virtualindex++;
      }
    }
    return undefined;
  },
  /**
   * @type {Number}
   * @readonly
   */
  length: new Property({
    get: function() {
      var filtered_len = 0;
      for (var i = 0, j = this.proxy_model.length; i < j; i++) {
        if (this.filter_fn(this.proxy_model.itemAtIndex(i)))
          filtered_len++;
      }
      return filtered_len;
    }
  }),
  /**
   * Fired after the model is changed.
   */
  didChangeModel: function(){
    this.notifyListeners();
  },
});

/**
 * Wraps the native array on {@link Model} interface.
 * @extend Model
 */
ArrayModel = Model.extend({

  /**
   * Initialize the content of model.
   *
   * If item is not an array, an array with one element will be created.
   *
   * @param {Array} array The initial content.
   * @event
   */
  init: function(array) {
    this.array = [].concat(array);
  },

  /** @inheritdoc */
  itemAtIndex: function(index) {
    return this.array[index];
  },
  /** @inheritdoc */
  length: new Property({
    get: function() { return this.array.length; }
  }),
  /** @inheritdoc */
  deleteItemAtIndex: function(index) {
    this.array.splice(index, 1);
    this.notifyListeners(index, 'delete');
  },
  /**
   * Updates the content of *item* at *index* with data.
   *
   * * If item at *index* and *data* are objects, item will be updated.
   * * Otherwise, item will be replaced with *data*.
   *
   * @param {Number} index
   * @param {Mixed} data
   */
  updateItemAtIndex: function(index, data) {
    var item = this.array[index];
    if (typeof(data) === 'object' && typeof(item) === 'object') {
      for (var i in data)
        if (data.hasOwnProperty(i))
          item[i] = data[i];
    } else {
      this.array[index] = data;
    }
    this.notifyListeners(index, 'update');
  },
  /**
   * Returns the index of data or null if not found.
   * @param {Mixed} data
   * @return {Number}
   */
  indexOf: function(data) {
    return this.array.indexOf(data);
  },
  /**
   * Pushes data to the model.
   * @param {Mixed} data
   * @param {Boolean} keepSelected
   */
  pushItem: function(data, keepSelected) {
    var oldSelectedIndex = this.selectedIndex;

    this.notifyListeners(this.array.push(data) - 1, 'insert');

    if (keepSelected)
      this.selectedIndex = oldSelectedIndex;
  }
});

/**
 * Adds filter feature to the {@link ArrayModel}.
 * See [Filter Example](#!/guide/filter)
 * @extend ArrayModel
 */
FilteredModel = ArrayModel.extend({
  /**
   * @inheritdoc
   * @event
   */
  init: function(array) {
    this.array = taffy();
    for (var i in array) {
      if(!array.hasOwnProperty(i))
        continue;
      this.array.insert({item: array[i]});
    }
  },
  /** @inheritdoc */
  itemAtIndex: function(index) {
    var item = this.array(this.filter).get()[index];
    return item && item.item;
  },
  /** @inheritdoc */
  length: new Property({
    get: function() { return this.array(this.filter).count(); }
  }),
  /**
   * Filter object to be applied on model.
   * @type {String}
   */
  filter: new Property({
    get: function() { return this.__filter; },
    set: function(filter) {
      if (filter) this.__filter = {item: filter};
      else delete this.__filter;
      this.notifyListeners();
    }
  }),
  /** @inheritdoc */
  deleteItemAtIndex: function(index) {
    var item = this.array(this.filter).get()[index];
    this.array(item['___id']).remove();
    this.notifyListeners(index, 'delete');
  },
  /** @inheritdoc */
  updateItemAtIndex: function(index, data) {
    var item = this.array(this.filter).get()[index];
    this.array(item['___id']).update({item: data});
    this.notifyListeners([index]);
  },
  /** @inheritdoc */
  pushItem: function(data) {
    this.array.insert({item: data});
    this.notifyListeners();
  }
});

/**
 * Models that represents the content of a filesystem hierarchy.
 * See [File Example](#!/guide/file)
 * @extend Model
 */
FileModel = Model.extend({
  /**
   * Initializes the model.
   *
   * Parameters are passed to this method through the constructor.
   * Uses the glob syntax on patterns to filter the model content.
   *
   * @param {String} path The initial path.
   * @param {String} patterns Patterns to be matched.
   * @param {Number} [depth] Internal use.
   * @event
   */
  init: function(path, patterns, depth) {
    this.path = path;
    this.patterns = patterns;
    this.array = [];

    var updateModel = function(entries) {
      /*
       * Only AFTER all leaves were visited, the tree is copied to
       * this.array. This process is assyncronous, thus at the first
       * time a controller tries to access the model, this.array
       * still be empty.
       */
      this.array = entries;
      this.notifyListeners();
      /*
       * Function 'updateModel' will be called as a callback, thus the context
       * of the FileModel object MUST be binded to it, otherwise
       * this.array will be undefined inside fs.listFiles execution.
       */
    }.bind(this);

    var sentinel = [];

    var fileModelTree = function(path, level) {
      var children = [];

      /* Every time a file is hit, its path is pushed in a sentinel array */
      sentinel.push(path);

      fs.listFiles(path, function(files, done, error) {
        for (var i = 0; i < files.length; ++i) {
          if (!files[i].isFile && level) {
            /*
             * The recursion continues inside folders until a certain
             * depth level. DEFAULT is TWO.
             */
            files[i].entries = fileModelTree(files[i].path, level - 1);
          }

          children.push(files[i]);
        }

        if (done) {
          /*
           * fs.listFiles will be called recursively, thus 'done == true'
           * every time a leaf is reached in the tree.
           */
          var idx = sentinel.indexOf(path);
          if (idx >= 0) {
            /*
             * When a leaf is hit, if the path is not the root, it is
             * remove from the sentinel array.
             */
            sentinel.splice(idx, 1);
            if (sentinel.length == 0) {
              /*
               * Thus when there is nothing in sentinel, it means that
               * all leafs of the tree were visited.
               */
              updateModel(root);
            }
          }
        }
      }, {filters: patterns});

      return children;
    };

    var root = fileModelTree(this.path, depth ? depth : 2);
  },
  /** @inheritdoc */
  itemAtIndex: function(index) {
    return this.array[index];
  },
  /** @inheritdoc */
  length: new Property({
    get: function() { return this.array.length; }
  })
});

/**
 * A persistent model of objects.
 * See [Notes Example](#!/guide/notes)
 * @extend Model
 */
DBModel = Model.extend({
  /**
   * Pre initialize the model.
   *
   * Prepare the {@link DBModel} to be initialized.
   *
   * @event
   * @param {String} database The database name.
   */
  willInitialize: function(database) {
    this._super();
    this.entries = taffy();
    this.entries.store(database);
  },
  /** @inheritdoc */
  itemAtIndex: function (index) {
    return this.entries().get()[index];
  },
  /** @inheritdoc */
  length: new Property({
    get: function() { return this.entries().count(); }
  }),
  /**
   * Updates the content of item at *index* with *values*.
   * @param {Number} index
   * @param {Object} values
   */
  updateItemAtIndex: function(index, values) {
    var item = this.itemAtIndex(index);
    this.entries(item['___id']).update(values);
    this.notifyListeners([index]);
  },
  /**
   * Inserts an object into model.
   * @param {Object} data
   */
  insert: function(data) {
    this.entries.insert(data);
    this.notifyListeners();
  },
  /**
   * Returns the index of the element that have the same
   * value on *key* attribute than *item*.
   * @param {Object} item
   * @param {String} [key='id']
   */
  indexOf: function(item, key) {
    var search = {};
    key = key || 'id';
    search[key] = item[key];
    return this.entries().get().indexOf(this.entries(search).first());
  },
  /** @inheritdoc */
  deleteItemAtIndex: function(index) {
    var item = this.itemAtIndex(index);
    this.entries(item['___id']).remove();
    this.notifyListeners();
  },
  /**
   * Removes all model content.
   */
  clear: function() {
    this.entries().remove();
    this.notifyListeners();
  }
});

/** @extend Class */
ActionSheet = Class.extend({
  /** @event */
  didInitialize: function() {
    this.content = elm.Box({
      expand: 'both',
      fill: 'both',
      resize: true,
      elements: {},
    });

    var count = 0;

    if (this.title)
      this.content.elements[count++] = elm.Label({ label: this.title });

    for (var i = 0; i < this.model.length; i++)
      this.content.elements[count++] = elm.Button({
        index: i,
        label: this.model[i],
        on_click: function() {
          this.inwin.visible = false;
          this.selectedItemAtIndex(this.index);
        }.bind(this)
      });

    this.inwin = EUI.window.elements.inwin;
    this.inwin.content = this.content;
    this.inwin.activate();
  }
});

/**
 *
 * @extends Controller
 * @private
 */
GenController = Controller.extend({

  /**
   * Implementations should return an object that describes the specified item.
   *
   * Properties might include:
   *
   *  * text
   *  * subtext
   *  * icon
   *  * end
   *
   * See [Photo Example](#!/guide/photo)
   *
   * @param {Number} index
   * @template
   */
  itemAtIndex: function(index) {},

  /**
   * Triggered when an item is selected.
   * @param {Number} index
   * @template
   * @event
   */
  selectedItemAtIndex: function(index) {},

  /**
   * @inheritdoc
   * @event
   */
  willInitialize: function(_type) {
    this._groups = {};
    this._type = _type;
    this.viewDescriptor = elm.Layout({
      horizontal: false,
      file: EUI.defaults._theme['list'],
      content: {
        search: elm.Entry({
          before: "list",
          scrollable: true, single_line: true,
          icon_visible: !!this.searchBarItems && this.searchBarItems.left,
          end_visible: !!this.searchBarItems && this.searchBarItems.right,
          content: {
            'icon': this.searchBarItems && this.searchBarItems.left,
            'end': this.searchBarItems && this.searchBarItems.right
          },
          on_change: function () {
            this.search(this.view.content.search.text);
          }.bind(this)
        }),
        list: _type({
          mode: this.mode ? this.mode : 'default',
          classes: {
            'default': {
              style: this.style,
              text: function(part, data) {
                var item = this._itemFromData(data.data);
                return item && item[part.replace('elm.', '').replace('.', '_')];
              }.bind(this),
              content: function(part, data) {
                var item = this._itemFromData(data.data);
                var image = item && item[part.replace('elm.swallow.', '')];
                return image && elm.Icon({ image: image });
              }.bind(this),
              state: function(part, data) {
                var item = this._itemFromData(data.data);
                return item ? item.state : null;
              }.bind(this)
            },
            'delete': {
              style: this.style,
              text: function(part, data) {
                var item = this._itemFromData(data.data);
                return item && item[part.replace('elm.', '').replace('.', '_')];
              }.bind(this),
              content: function(part, data) {
                if (part == 'elm.swallow.end')
                  return elm.Button({ label: "Delete"});
                var item = this._itemFromData(data.data);
                var image = item && item[part.replace('elm.swallow.', '')];
                return image &&  elm.Icon({ image: image });
              }.bind(this),
              state: function(part, data) {
                var item = this._itemFromData(data.data);
                return item && item.state;
              }.bind(this)
            },
            'group': {
              style: 'group_index',
              text: function(part, data) {
                var item = this._itemFromData(data.data);
                return item && item[part.replace('elm.', '').replace('.', '_')];
              }.bind(this)
            },
            'loading': {
              style: 'default',
              text: function() {
                return 'Loading';
              },
              content: function() {
                return elm.Icon({ image: 'refresh' });
              }
            },
            'full': {
              style: 'full',
              content: function(part, data) {
                var item = this._itemFromData(data.data);
                return item && item.content;
              }.bind(this)
            }
          },
          elements: {}
        })
      }
    });
  },
  /** @private */
  _fetchItemUsingCache: function(index) {
    if (this._cachedItem && this._cachedItem.model_index === index)
      return this._cachedItem;

    var cachedItem = this.itemAtIndex(index);
    if (cachedItem) {
      cachedItem.model_index = index;

      this._cachedItem = cachedItem;
      return cachedItem;
    }

    return {};
  },
  /** @private */
  _itemFromData: function(data) {
    if (data.model_index !== undefined)
      return this._fetchItemUsingCache(data.model_index);

    if (data.group !== undefined)
      return { text: data.group };

    return {};
  },
  /**
   * @inheritdoc
   * @event
   */
  didInitialize: function(_type) {
    if (this.editable) {
      if (!this.navigationBarItems.right)
        this.navigationBarItems.right = 'Add';
      this.viewDescriptor.content.list.on_longpress = function (item) {
        item['class'] = this.view.content.list.classes['delete'];
      }.bind(this);
    }
    this._super();
  },
  /** @private */
  updateItemAtIndex: function(index) {
    var item = this._fetchItemUsingCache(index);
    if (!item)
      return;

    var view = this.view.content.list;
    var data = { model_index: index };

    if (item.group !== undefined) {
      var group = this._groups[item.group];
      if (!group) {
        view.elements[item.group] = {
          before: index,
          class: view.classes.group,
          data: { group: item.group },
          group: true
        };

        this._groups[item.group] = 0;
      }

      data.group = item.group;
      this._groups[item.group]++;
    }

    var element = {
      class: view.classes[item.style || 'default'],
      data: data,
      on_select: function(item) {
        if (typeof(this.selectedItemAtIndex) === 'function')
          this.selectedItemAtIndex(item.data.model_index);
      }.bind(this)
    };

    if (item.group !== undefined)
      element.parent = view.elements[item.group];

    view.elements[index] = element;
  },
  /** @private */
  _removeIndexesFromView: function(view, indexes) {
    /*
     * FIXME: This is far from ideal in the case where there are
     * groups. Unfortunately item datas might get unsync'd. This
     * shouldn't be a problem when not using groups, so just resort
     * to the good ol' way of deleting items without recreating
     * the whole list in that case.
     */
    indexes = [].concat(indexes);

    if (Object.keys(this._groups).length > 0) {
      for (var index in indexes) {
        var item = view.elements[indexes[index]];
        this._groups[item.data.group]--;
        if (!this._groups[item.data.group]) {
          delete view.elements[item.data.group];
          delete this._groups[item.data.group];
        }
      }

      view.clear();
      this.updateView();
    } else {
      view.clear(indexes);
    }
  },
  /** @private */
  _updateIndexes: function(indexes) {
    indexes = [].concat(indexes);

    for (var i = 0; i < indexes.length; i++)
      this.updateItemAtIndex(indexes[i]);
  },
  /** @private */
  _updateAllIndexes: function(view, indexes) {
    this._groups = {};
    for (var i = 0, j = this.model.length; i < j; ++i) {
      this.updateItemAtIndex(i);
    }

    while (view.elements[i] !== undefined) {
      delete view.elements[i];
      i++;
    }
  },
  /**
   * @inheritdoc
   * @event
   */
  updateView: function(indexes, hint) {

    this.searchBarVisible = !!this.search;

    if (hint === 'select')
      return;

    var view = this.view.content.list;

    if (hint === 'delete') {
      this._removeIndexesFromView(view, indexes);
      return;
    }

    if (indexes !== undefined) {
      this._updateIndexes(indexes);
      return;
    }

    this._updateAllIndexes(view, indexes);
  },

  /**
   * State of search bar visibility.
   */
  searchBarVisible: new Property({
    get: function() { return this._cachedSearchBarVisible; },
    set: function(setting) {
      if (setting === this._cachedSearchBarVisible)
        return;
      this._cachedSearchBarVisible = setting;
      this.view.signal_emit(setting ? "show,search" : "hide,search", "");
    }
  })

});

/**
 * Shows a searchable list with model content.
 * @extend GenController
 */
ListController = GenController.extend({
  /**
   * Fired before #init is called.
   * @event
   */
  willInitialize: function() {
    this._super(elm.Genlist);
    this.contextMenuDirection = 'horizontal';
    this.listening_scroll = true;
  },
  /**
   * Fired after #init is called.
   * @event
   */
  didInitialize: function() {
    this._super();
    var list = this.viewDescriptor.content.list;
    list.on_longpress = this.on_longpress.bind(this);
    list.on_scrolled_over_top_edge = this.on_scrolled_over_edge.bind(this, "top");
    list.on_scrolled_over_bottom_edge = this.on_scrolled_over_edge.bind(this, "bottom");
    list.on_scrolled_over_left_edge = this.on_scrolled_over_edge.bind(this, "left");
    list.on_scrolled_over_right_edge = this.on_scrolled_over_edge.bind(this, "right");
  },
  /** @private */
  on_longpress: function(item) {
    if (!this.contextMenuItems)
      return;

    var menuItems;
    if (typeof(this.contextMenuItems) === 'function')
      //item.data.model_index is the item index on the list
      menuItems = this.contextMenuItems(item.data.model_index);
    else
      menuItems = this.contextMenuItems;

    if (menuItems) {
      this.viewContent.content.contextMenu = elm.CtxPopup({
        visible: false,
        horizontal: this.contextMenuDirection == 'horizontal',
        on_item_select: function(item) {
          if (this.selectedContextMenuItem)
            this.selectedContextMenuItem(item, this.viewContent.content.contextMenu.listIndex);
          this.viewContent.content.contextMenu.dismiss();
        }.bind(this),
      });

      var ctxMenu = this.viewContent.content.contextMenu;
      ctxMenu.elements = [];
      for (i in menuItems) {
        if (typeof(menuItems[i]) === 'object') {
          ctxMenu.elements[i] = {};
          for (var key_index = 0, keys = Object.keys(menuItems[i]), keys_length = keys.length;
                 key_index < keys_length; ++key_index)
            ctxMenu.elements[i][keys[key_index]] = menuItems[i][keys[key_index]];

          ctxMenu.elements[i].text = menuItems[i].label;
          ctxMenu.elements[i].icon = EUI.widgets.Icon({image: menuItems[i].icon,
              resizable_up: false, resizable_down: false})
        }
        else
          ctxMenu.elements[i] = menuItems[i];
      }

      ctxMenu.hover_parent = EUI.window;
      ctxMenu.x = this.view.content.list.pointer.x;
      ctxMenu.y = this.view.content.list.pointer.y;
      ctxMenu.listIndex = item.data.model_index;
      ctxMenu.show();
    }
  },
  /** @private */
  on_scrolled_over_edge: function(edge) {
    if (this.listening_scroll && typeof(this.didScrollOverEdge) === 'function')
      this.didScrollOverEdge(edge);
  },
  /**
   * @inheritdoc
   * @event
   */
  updateView: function(indexes, hint) {
    if (hint == 'beginSlowLoad') {
      var edge = indexes == -1 ? 'top' : 'bottom';
      this._showLoadingItem(edge);
      return;
    }

    if (hint == 'finishSlowLoad') {
      var edge = indexes == -1 ? 'top' : 'bottom';
      this._hideLoadingItem(edge);
      return;
    }

    this._super(indexes, hint);
  },
  /** @private */
  _ignoreScrollEvents: function() {
    this.listening_scroll = false;
    setTimeout(function() {
      this.listening_scroll = true
    }.bind(this), 600);
  },
  /** @private */
  _showLoadingItem: function(edge){
    var view = this.view.content.list;
    var loading_element = {
      class: view.classes.loading
    };

    if (edge == 'top')
      loading_element.before = 0;

    this._ignoreScrollEvents();
    view.elements[edge] = loading_element;
    view.bring_in_item(view.elements[edge], "in");
 },
  /** @private */
  _hideLoadingItem: function(edge) {
    var view = this.view.content.list;
    if (view.elements[edge])
      delete view.elements[edge];

    //to prevent 'edge,*' signals from messing up around here
    this._ignoreScrollEvents();
  }
});

/**
 * Shows a grid with the model content.
 * @extend GenController
 */
GridController = GenController.extend({
  /**
   * Fired before #init is called.
   * @event
   */
  willInitialize: function() {
    this._super(elm.Gengrid);
  },
  /**
   * Fired after #init is called.
   * @event
   */
  didRealizeView: function() {
    this.view.content.list.item_size_vertical = 128;
    this.view.content.list.item_size_horizontal = 128;
    this._super();
  }
});

/**
 * Web viewer controller.
 * @extend Controller
 */
var WebController = Controller.extend({
  /** @inheritdoc */
  toolbarItems: [
    { tag: 'back', icon: 'go-previous', label: 'Back' },
    { tag: 'forward', icon: 'go-next', label: 'Forward' },
    { tag: 'stop-reload',
      states: {
        idle: { tag: 'reload', icon: 'reload', label: 'Reload' },
        loading: { tag: 'stop', icon: 'process-stop', label: 'Stop' }
      }
    },
    {
      element: elm.Entry({
        fill: 'both',
        expand: 'both',
        single_line: true,
        scrollable: true,
        hint_min: {
          width: 250,
          height: 30
        }
      })
    }
  ],

  /**
   * @inheritdoc
   * @event
   */
  selectedToolbarItem: function(item) {
    switch (item.tag) {
    case 'back':
      this.goBack();
      break;
    case 'forward':
      this.goForward();
      break;
    case 'reload':
      this.reload();
      break;
    case 'stop':
      this.stop();
      break;
    }
  },

  /**
   * @inheritdoc
   * @event
   * @param {String} url Initial URL
   */
  willInitialize: function(url) {
    url = url || this.url || 'about:blank';
    this.title = url;

    var progress = elm.Notify({
      content: elm.ProgressBar({ value: 0.0 }),
      visible: false,
      orient: 'top-right'
    });

    var link_hover_notify = elm.Notify({
      content: elm.Label({}),
      visible: false,
      orient: 'bottom-left'
    });

    var web = elm.Web({
      expand: 'both',
      fill: 'both',
      history_enabled: true,
      uri: url,

      on_load_progress: function(progress) {
        if (progress < 1.0) {
          this.progress.visible = true;
          this.progress.content.value = progress;
        } else {
          this.progress.visible = false;
        }

        if (this.stop_reload_button)
          this.stop_reload_button.state = this.progress.visible ? 'loading' : 'idle';

        if (this.didChangeProgress)
          this.didChangeProgress(progress);
      }.bind(this),

      on_title_change: function(title) {
        this.title = title || this.web.uri;
        this.evaluateViewChanges();

        if (this.didChangeTitle)
          this.didChangeTitle(title);
      }.bind(this),

      on_uri_change: function(uri) {
        this.url_entry.text = uri;

        if (this.toolbar) {
          this.toolbar.elements[0].enabled = this.web.back_possible;
          this.toolbar.elements[1].enabled = this.web.forward_possible;
        }

        if (this.didChangeURI)
          this.didChangeURI(uri);
      }.bind(this),

      on_link_hover_in: function(uri, title) {
        this.link_hover_notify.visible = true;
        this.link_hover_notify.content.text = uri;
      }.bind(this),

      on_link_hover_out: function() {
        this.link_hover_notify.visible = false;
      }.bind(this)
    });

    this.viewDescriptor = elm.Box({
      expand: 'both',
      fill: 'both',
      horizontal: false,
      elements: {
        web: web,
        progress: progress, /* FIXME get this out of the box */
        link_hover_notify: link_hover_notify /* FIXME get this out of the box */
      },
    });
  },

  /**
   * @inheritdoc
   * @event
   */
  updateView: function() {
    var view = this.view;

    if (!view)
      return;

    if (!this.web) {
      this.progress = view.elements.progress;
      this.web = view.elements.web;
      this.link_hover_notify = view.elements.link_hover_notify;
    }
  },

  /** @private */
  goBack: function() {
    this.web.back();
  },

  /** @private */
  goForward: function() {
    this.web.forward();
  },

  /** @private */
  reload: function() {
    this.web.reload();
  },

  /** @private */
  stop: function() {
    this.web.stop();
  },

  /** @private */
  go: function(uri) {
    this.link_hover_notify.visible = false;
    this.progress.visible = false;
    this.web.uri = uri;
  },

  /**
   * Fired after toolbar change.
   * @event
   */
  didUpdateToolbar: function(toolbar, items) {
    this.toolbar = toolbar;
    this.url_entry = toolbar.elements[3].element;
    this.stop_reload_button = toolbar.elements[2];

    this.stop_reload_button.state = 'idle';

    this.url_entry.on_activate = function() {
      var url = this.url_entry.text;

      if (url.substr(0, "http://".length) != "http://" ||
          url.substr(0, "https://".length) != "https://")
        url = "http://" + url;

      this.go(url);
    }.bind(this);
  },

  /**
   * Fired after {@link Controller} was popped.
   * @event
   */
  didPopController: function() {
    this.web.on_link_hover_in = null;
    this.web.on_link_hover_out = null;
    this.web.on_load_progress = null;
    this.link_hover_notify.visible = false;
    this.progress.visible = false;
  }
});

/**
 * Container base class.
 *
 * Manage a model of {@link Controller}s.
 *
 * @extend Controller
 */
Container = Controller.extend({
  /**
   * @inheritdoc
   * @protected
   */
  evaluateViewChanges: function() {},
  /** @private */
  _getViewDescriptor: function() {
    return this.viewDescriptor;
  },
  /**
   * @inheritdoc
   * @protected
   */
  _getWrappedViewDescriptor: function() {
    return this._getViewDescriptor();
  },
  /**
   * @inheritdoc
   * @protected
   */
  _setViewContent: function(view) {
    throw "It's a 'Container' and can't be pushed in a regular 'Controller'";
  },
  /**
   * @inheritdoc
   * @protected
   */
  _setRealizedApp: function(realized) {
    this._view = realized;
    this.didRealizeView();
  },
  /** @inheritdoc */
  view: new Property({
    get: function() { return this._view; },
  }),
  /**
   * @inheritdoc
   * @event
   */
  didRealizeView: function () {
    if (this.model)
      this.model.addListener(this);
    this.updateView();
  },
  /**
   * Promote the controller at the specified index to the top of the stack.
   * @type {Number} index
   */
  promoteController: function(index) {
    this.updateView(index, 'select');
  }
});

/**
 * Show only the specified {@link Controller}.
 * @inheritdoc
 * @extend Container
 */
FrameController = Container.extend({
  /**
   * @inheritdoc
   * @event
   */
  willInitialize: function() {
    this.viewDescriptor = elm.Naviframe({
      title_visible: false,
      elements: {}
    });
  },
  /**
   * @inheritdoc
   * @event
   */
  updateView: function(index, hint) {

    var ctrl = this.model.itemAtIndex(index || 0);
    if (!ctrl) return;

    var view = this.view;

    if (ctrl._isRealized) {
      var elements = view.elements;
      for (var i in elements)
        if (ctrl._isEqual(elements[i].content))
          elements[i].promote();
    } else {
      var len = Object.keys(view.elements).length;
      ctrl.parent = this;
      view.elements[len] = { content: ctrl._getWrappedViewDescriptor() };
      ctrl._setRealizedApp(view.elements[len].content);
    }
  },
});

/**
 * Split view in two or more {@link Controller}s.
 *
 * The most left {@link Controller} can be hidden.
 *
 * @inheritdoc
 * @extend Container
 */
SplitController = Container.extend({
  /**
   * @inheritdoc
   * @event
   */
  willInitialize: function() {
    this.viewDescriptor = elm.Layout({
      expand: 'both',
      fill: 'both',
      resize: true,
      file: EUI.defaults._theme['split'],
      content: {}
    });
  },
  /**
   * @inheritdoc
   * @event
   */
  updateView: function(index, hint) {
    var view = this.view;
    var panels = ['left', 'right'];
    var len = Math.min(this.model.length, panels.length);

    switch (EUI.uiMode) {
    case 'tablet':
      this.view.signal_emit("tablet,mode", "");
      break;
    case 'phone':
      this.view.signal_emit("phone,mode", "");
      break;
    }

    for (var i = 0; i < len; i++) {
      var ctrl = this.model.itemAtIndex(i);
      var panel = panels[i];

      if (ctrl._isRealized && ctrl._isEqual(view.content[panel]))
        continue;

      this[panel] = ctrl;
      ctrl.parent = this;

      var app = ctrl._getWrappedViewDescriptor();
      delete app.resize;
      delete app.expand;
      delete app.fill;

      view.content[panel] = app;
      ctrl._setRealizedApp(view.content[panel]);
    }
  },

  /**
   * Left panel visibility state.
   */
  leftPanelVisible: new Property({
    set: function(setting) {
      if (EUI.uiMode !== 'tablet')
        this.view.signal_emit(setting ? "show,left" : "hide,left", "");
    }
  })
});

/**
 * Toolbar controller.
 *
 * @inheritdoc
 * @extend Container
 */
ToolController = Container.extend({
  /**
   * @inheritdoc
   * @event
   */
  willInitialize: function() {
    this.viewDescriptor = elm.Toolbar({
      shrink_mode: 'expand',
      select_mode: 'always',
      elements: {}
    });
  },

  /**
   * Fired when a item is selected.
   * @param {Number} index
   */
  selectedItemAtIndex: function(index) {},

  /**
   * @inheritdoc
   * @event
   */
  updateView: function(index, hint) {

    var elements = this.view.elements;
    var selected = this.model.selectedIndex;

    var update = function(index) {

      var real = elements[index];

      if (!real) {
        elements[index] = {};
        real = elements[index];
      }

      real.on_select = this.selectedItemAtIndex.bind(this, index);
      real.selected = (index === selected);

      var ctrl = this.model.itemAtIndex(index);
      real.label = ctrl.title;
      real.icon = ctrl.icon;

    }.bind(this);

    /* Update controllers items */
    if (index !== undefined)
      update(index);
    else
      for (var len = this.model.length, index = 0; index < len; index++)
        update(index);

    /* Update toolbar items */
    var items = this.toolbarItems;
    if (items.length) {
      var len = this.model.length;
      elements[len] = { separator: true };
      for (var i = 0; i < items.length; i++) {
        elements[len + i] = items[i];
        elements[len + i].on_select = this.selectedToolbarItem.bind(this, elements[len + i]);
      }
    }
  }
});

/**
 * Shows a toolbar and the selected controller.
 * @extend Container
 */
TabController = Container.extend({
  /**
   * @inheritdoc
   * @event
   */
  willInitialize: function() {
    this.viewDescriptor = elm.Layout({
      expand: 'both',
      fill: 'both',
      resize: true,
      file: EUI.defaults._theme['app'],
      content: {}
    });
  },

  /**
   * @inheritdoc
   * @event
   */
  didRealizeView: function() {

    this.toolbar = new ToolController();
    this.toolbar.model = this.model;
    this.toolbar.toolbarItems = this.toolbarItems;
    this.toolbar.selectedToolbarItem = this.selectedToolbarItem;
    this.toolbar.selectedItemAtIndex = function(index) {
      this.model.selectedIndex = index;
    };

    var view = this.view;
    view.content.toolbar = this.toolbar._getWrappedViewDescriptor();
    this.toolbar._setRealizedApp(view.content.toolbar);

    this.frame = new FrameController();
    this.frame.parent = this;
    this.frame.model = this.model;

    view.content.view = this.frame._getWrappedViewDescriptor();
    this.frame._setRealizedApp(view.content.view);

    this.model.addListener(this);

    if (this.model.length)
      this.model.selectedIndex = 0;
  },

  /**
   * @inheritdoc
   * @event
   */
  updateView: function(index, hint) {

    var view = this.view;
    view.signal_emit(this.tabPosition + ',toolbar', '');
    view.signal_emit((this.hasTabBar == false ? 'hide' : 'show') + ',toolbar', '');
  },
});

/**
 * Image viewer controller.
 *
 * @extend Controller
 */
ImageController = Controller.extend({
  /**
   * @inheritdoc
   * @event
   */
  willInitialize: function() {
    this.viewDescriptor = elm.Photocam({
      expand: 'both',
      fill: 'both',
      zoom_mode: "auto-fill",
      zoom: 5.0
    });
  },
  /** @inheritdoc */
  navigationBarStyle: 'overlap',
  /**
   * @inheritdoc
   * @event
   */
  didRealizeView: function() {
    this.view.on_click = function() {
      if (this.didClickView)
        this.didClickView();
    }.bind(this);
    this._super();
  },
  /** @inheritdoc Model#length */
  length: new Property({
    get: function () { return this.model.length; }
  }),
  /**
   * Specifies the image to be shown.
   * @param {Number} index
   */
  setImage: function(index) {
    if ((index < 0) || (index >= this.length))
      return;
    this.index = index;
    this.didChangeModel();
  },
  /**
   * @inheritdoc
   * @event
   */
  updateView: function() {
    this.view.file = this.model.itemAtIndex(this.index).path;
  },
});

/**
 * Video viewer controller.
 * @extend Controller
 */
VideoController = Controller.extend({
  /**
   * @inheritdoc
   * @event
   */
  willInitialize: function() {
    this.viewDescriptor = elm.Box({
      expand: 'both',
      fill: 'both',
      horizontal: false,
      elements: {
        video: elm.Video({
          expand: 'both',
          fill: 'both',
        }),
        controllers: elm.VideoControllers({

        })
      }
    });
  },
  /** */
  playerBar : ['Play', 'Pause'],
  /**
   * @type {Number}
   * @readonly
   */
  length: new Property({
    get: function () { return this.model.length; }
  }),
  /**
   * @param {Number} index
   */
  setVideo: function(index) {
    if ((index < 0) || (index >= this.length))
      return;
    this.index = index;
    this.didChangeModel();
  },
  /** */
  itemAtIndex : function(index){
    return this.model.itemAtIndex(this.index);
  },
  /**
   * @inheritdoc
   * @event
   */
  updateView: function() {
    var elements = this.view.elements;
    if (!elements.controllers.video)
      elements.controllers.video = elements.video;

    elements.video.file = this.itemAtIndex(this.index);
  },
});

/**
 * Base controller for {@link Widgets}.
 * @extend Controller
 */
TableController = Controller.extend({
  /**
   * Bidimensional array to positioning {@link Widgets} in the view.
   * @type {Array}
   */
  fields: [[]],
  /**
   * @inheritdoc
   * @event
   */
  willInitialize: function() {
    this.viewDescriptor = elm.Table({
      expand: 'both',
      fill: 'both',
      elements: {}
    });
  },
  /**
   * @inheritdoc
   * @event
   */
  updateView: function(index, hint) {

    var elements = this.view.elements;
    var record = this.model && this.model.itemAtIndex(this.index) || {};

    for (var row in this.fields) {
      for (var col in this.fields[row]) {
        var item = this.fields[row][col];
        if ('V>'.indexOf(item.toString()) >= 0)
          continue;

        row = Number(row);
        col = Number(col);

        var id = item.field || [col,row].join();
        var real = elements[id] && elements[id].element;

        if (!real) {

          for (var i = col + 1; this.fields[row][i] == '>'; i++);
          var colspan = i - col;

          for (var i = row + 1; this.fields[i] && this.fields[i][col] == 'V'; i++);
          var rowspan = i - row;

          elements[id] = {
            col: col,
            row: row,
            colspan: colspan,
            rowspan: rowspan,
            element: item
          };

          real = elements[id].element;

          for (var i in real.event_map)
            if (typeof(this[real.event_map[i]]) === 'function')
              real[i] = this[real.event_map[i]].bind(this, real);
        }

        if (item.field) {
          real.setValue(record[item.field]);
          real.editable = this.editable && (item.editable !== false);
        } else {
          real.editable = false;
        }
      }
    }
  },
  /**
   * Returns a copy of the values shown on view.
   * @return {Object}
   */
  getValues: function() {
    var values = {};
    var elements = this.view.elements;
    for (var key in elements) {
      var real = elements[key].element;
      if (real && real.field)
        values[key] = real.getValue();
    }
    return values;
  },
  /**
   * Selected item index.
   */
  index: new Property({
    get: function() { return this.__index; },
    set: function(value) {
      if (!this.model) throw "Model must be defined before index";
      if (value >= 0 && value < this.model.length)
        this.__index = value;
    }
  })
});

/**
 * Extends {@link TableController} to automate insert and update actions on model..
 * @extend TableController
 */
FormController = TableController.extend({
  /**
   * @inheritdoc
   * @property
   */
  navigationBarItems: function() {
    return this.editable && {
      left: 'Cancel',
      right: (this.index === undefined) ? 'Add' : 'Save'
    };
  },
  /**
   * @inheritdoc
   * @event
   */
  selectedNavigationBarItem: function(item) {
    switch (item) {
      case 'Add':
        this.model.insert(this.getValues());
        break;
      case 'Save':
        this.model.updateItemAtIndex(this.index, this.getValues());
        break;
    }
    this.popController();
  },
});

/**
 * @extend Class
 * @singleton
 * @private
 */
RoutingSingleton = Class.extend({
  /** */
  database: {
    edit: {
      'image/*': [
        { name: 'Photo Viewer', app: 'photo.js' }
      ],
      'text/*': [
        { name: 'Notepad', app: 'notepad.js' }
      ]
    },
    view: {
      'image/*': [
        { name: 'Photo Viewer', app: 'photo.js' }
      ],
      'text/*': [
        { name: 'Notepad', app: 'notepad.js' }
      ],
      'text/html': [
        { name: 'Web Browser', app: 'browser.js' }
      ]
    },
    share: {
      '*': [
        { name: 'Mail', app: 'mail.js' },
        { name: 'Send over Bluetooth', app: 'bluetooth-send.js' }
      ]
    }
  },

  /** */
  possibleActions: function() {
    return Object.keys(this.database);
  },

  /**
   * @param {String} mime_type
   * @param {String} file
   */
  possibleActionsForType: function(action, mime_type) {
    var handlers = this.database[action];
    if (handlers === undefined)
      return [];

    var possible_handlers = [];
    for (var handler in handlers) {
      handler = handlers[handler];

      if (handler[0].exec(mime_type) !== null)
        possible_handlers = possible_handlers.concat(handler[1]);
    }

    return possible_handlers;
  },

  /**
   * @param {String} mime_type
   * @param {String} file
   */
  edit: function(mime_type, file) {
    return this.performAction('edit', mime_type, file);
  },

  /**
   * @param {String} mime_type
   * @param {String} file
   */
  share: function(mime_type, file) {
    return this.performAction('share', mime_type, file);
  },

  /**
   * @param {String} mime_type
   * @param {String} file
   */
  view: function(mime_type, file) {
    return this.performAction('view', mime_type, file);
  },

  /**
   * @param {String} action
   * @param {String} mime_type
   * @param {String} file
   */
  performAction: function(action, mime_type, file) {
    var possible_handlers = this.possibleActionsForType(action, mime_type);

    if (possible_handlers.length == 1) {
      print("Only one possible handler to", action,
              file, "(", mime_type, "):", possible_handlers);
    } else if (possible_handlers.length > 1) {
      print("Multiple possible handlers to", action,
              file, "(", mime_type, "):", possible_handlers);
    } else {
      print("No handlers to", action, "mime_type", mime_type);
      return false;
    }

    return true;
  },

  /** */
  init: function() {
    var database = {};

    for (var action in this.database) {
      if (!this.database.hasOwnProperty(action))
        continue;
      database[action] = [];

      for (var mime_type in this.database[action]) {
        if (!this.database[action].hasOwnProperty(mime_type))
          continue;

        var glob_as_regex = new RegExp(mime_type.replace("*", ".*"));

        database[action].push([
          glob_as_regex, this.database[action][mime_type]
        ]);
      }
    }

    this.database = database;
  }
});

/** @singleton */
Routing = {
  __instance__: new RoutingSingleton(),
  edit: function(mime_type, file) {
    return Routing.__instance__.edit(mime_type, file);
  },
  view: function(mime_type, file) {
    return Routing.__instance__.view(mime_type, file);
  },
  share: function(mime_type, file) {
    return Routing.__instance__.share(mime_type, file);
  },
  possibleActions: function(action, mime_type) {
    if (action !== undefined && mime_type !== undefined)
      return Routing.__instance__.possibleActionsForType(action, mime_type);
    return Routing.__instance__.possibleActions();
  },
  performAction: function(action, mime_type, file) {
    return Routing.__instance__.performAction(action, mime_type, file);
  }
};

EUI = exports;

exports.controllers = [];

exports.loadingState = 0;

/** */
exports.uiMode = (function() {
  if (environment['EASYUI_UI_MODE'] === 'tablet')
    return 'tablet';
  return 'phone';
})();

exports.defaults = utils.clone(defaults);
exports.defaults._theme = exports.defaults.themes[exports.defaults.theme];

/**
 * Overwrite EUI.defaults values.
 * @param {Object} options
 */
exports.setDefaults = function(options) {
  if (options) {
    var keys = Object.keys(options);
    for (var i = 0; i < keys.length; ++i)
      exports.defaults[keys[i]] = options[keys[i]];
  }
};

exports.app = function(app) {

  if (!(app instanceof Controller))
    throw "First argument must be a Controller instance.";

  elm.addThemeExtension('./themes/default.edj');

  EUI.window = elm.realise(elm.Window({
    width: 320,
    height: 480,
    title:  app.title || EUI.defaults.title,
    on_delete: function() { EUI.__shutting_down = true },
    elements: {
      'background': elm.Background({
        expand: 'both',
        fill: 'both',
        resize: true,
      }),
      'inwin': elm.Inwin({
        visible: false,
      }),
      'notify': elm.Notify({
        visible: false,
        orient: 'center',
        expand: 'both',
        fill: 'both',
        content: elm.ProgressBar({
          value: 0.0,
          style: 'wheel'
        })
      })
    }
  }));

  EUI.window.elements.app = app._getWrappedViewDescriptor();
  app._setRealizedApp(EUI.window.elements.app);
};

/** */
exports.setLoadingState = function(state) {
  var blockUI = function() {
    if (!EUI.window) {
      setTimeout(blockUI, 50);
      return;
    }

    if (EUI.window.elements.notify.visible)
      return;

    EUI.window.elements.notify.visible = true;
    EUI.window.elements.notify.content.pulse(true);
    EUI.window.elements.app.signal_emit("block,ui", "");
  }
  var unblockUI = function() {
    if (!EUI.window.elements.notify.visible)
      return;

    EUI.window.elements.notify.visible = false;
    EUI.window.elements.notify.content.pulse(false);
    EUI.window.elements.app.signal_emit("unblock,ui", "");
  }

  if (state)
    ++EUI.loadingState;
  else
    --EUI.loadingState;

  if (EUI.loadingState < 0)
    EUI.loadingState = 0;

  if (EUI.loadingState > 0)
    blockUI();
  else
    unblockUI();
};

exports.DBModel = DBModel;

exports.FileModel = FileModel;

exports.FilteredModel = FilteredModel;
exports.FilterModel = FilterModel;
exports.ArrayModel = ArrayModel;
exports.Routing = Routing;

exports.Model = function(proto) {
  return Model.extend(proto);
};

exports.ListController = function(proto) {
  return ListController.extend(proto);
};

exports.GridController = function(proto) {
  return GridController.extend(proto);
};

exports.ActionSheet = function(proto) {
  return ActionSheet.extend(proto);
};

exports.ImageController = function(proto) {
  return ImageController.extend(proto);
};

exports.TabController = function(proto) {
  return TabController.extend(proto);
};

exports.FormController = function(proto) {
  return FormController.extend(proto);
};

exports.FrameController = function(proto) {
  return FrameController.extend(proto);
};

exports.TableController = function(proto) {
  return TableController.extend(proto);
};

exports.SearchController = function(proto) {
  return SearchController.extend(proto);
};

exports.SplitController = function(proto) {
  return SplitController.extend(proto);
};

exports.WebController = function(proto) {
  return WebController.extend(proto);
};

exports.VideoController = function(proto) {
  return VideoController.extend(proto);
};

exports.widgets = undefined;

/**
 * @class
 */
var Widgets = {};
exports.widgets = Widgets;

var wrapElm = function(widget, _default) {
  return function(proto) {
    proto.expand = proto.expand || 'both';
    proto.fill = proto.fill || 'both';
    proto.event_map = proto.event_map || _default.event_map || {};
    for (var i in _default)
      if (proto[i] == undefined)
        proto[i] = _default[i];
    return widget(proto);
  }
};

/** @method */
Widgets.Button = wrapElm(elm.Button, {
  event_map: {
    on_click: 'didClickOnElement',
    on_press: 'didPressOnElement',
    on_release: 'didReleaseElement'
  },
  setValue: function (value) { if (this.text != value) this.text = value },
  getValue: function() { return this.text }
});

/** @method */
Widgets.Label = wrapElm(elm.Label, {
  setValue: function (value) { this.file = value; }
});

/** @method */
Widgets.Entry = wrapElm(elm.Entry, {
  event_map: {
    on_activate: 'didActivate',
    on_change: 'didChangeEntry'
  },
  setValue: function (value) {
    value = (value !== undefined) ? value : '';
    if (this.text != value)
      this.text = this.utf8_to_markup(value);
  },
  getValue: function() { return this.markup_to_utf8(this.text) }
});

/** @method */
Widgets.Photocam = wrapElm(elm.Photocam, {
  setValue: function(value) { this.file = value },
  getValue: function() { return this.file }
});

/** @method */
Widgets.Web = wrapElm(elm.Web, {
  type: 'Web',
  event_map: {
    on_load_progress: 'didChangeProgress',
    on_ready: 'didFinishLoading',
    on_title_change: 'didChangeTitle',
    on_uri_change: 'didChangeUri',
  },
});

/** @method */
Widgets.Icon = wrapElm(elm.Icon, {
  setValue: function (value) { if (this.file != value) this.file = value },
  getValue: function() { return this.file }
});

/** @method */
Widgets.Check = wrapElm(elm.Check, {
  setValue: function(value) { this.state = !!value },
  getValue: function() { return this.state }
});

/** @method */
Widgets.Table = wrapElm(elm.Table, {});

/** @method */
exports.module_hooks = {
  'ajax': {
    beforeSend: function(request, options) {
      if (options.blockUI)
        EUI.setLoadingState(true);
    },
    onComplete: function(request, options) {
      if (options.blockUI)
        EUI.setLoadingState(false);
    }
  }
};
