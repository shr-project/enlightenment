Class = require('class').Class;
elm = require('elm.so');
fs = require('fs');
taffy = require('taffy').taffy;

Controller = Class.extend({
  init: function(kwargs) {
    var model = this._feature(this.model);
    if (model)
      model.addController(this);
  },

  _feature: function(feature, defaultValue) {
    if (feature === undefined)
      return defaultValue;
    if (typeof(feature) === 'function')
      return feature.apply(this);
    return feature;
  },

  _getNavigationBarItems: function() {
    return this._feature(this.navigationBarItems, {});
  },

  _getToolBarItems: function() {
    return this._feature(this.toolbarItems, []);
  },

  _getTitle: function(defaultTitle) {
    return this._feature(this.title, null);
  },

  _setViewContent: function(view) {
    this.viewContent = view.content;
    this.naviframe_item = view;
  },

  _getView: function() {
    return this.viewContent.content.view;
  },

  _getViewDescriptor: function() {
    if (this.cachedViewDescriptor !== undefined)
      return this.cachedViewDescriptor;

    this.cachedViewDescriptor = elm.Layout({
      file: { name: 'eui.edj', group: 'app' },
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

  popController: function() {
    this.naviframe.pop();
  },

  _shutdown: function() {
    if (EUI.__shutting_down)
      return;

    if (this.willPopController)
      this.willPopController();

    if (this.model)
      this.model.removeController(this);

    if (this.parent)
      this.parent.evaluateViewChanges();
  },

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
      style: ctrl._feature(ctrl.navigationBarStyle, null),
      on_delete: ctrl._shutdown.bind(ctrl),
    };

    ctrl._setViewContent(ctrl.naviframe.elements[cnt]);
    ctrl.didRealizeView();
  },

  _realizeApp: function() {
    return elm.Naviframe({
      expand: 'both',
      fill: 'both',
      resize: true,
      title_visible: false,
      elements: [
        {
          'content': this._getViewDescriptor(),
          'on_delete': this._shutdown.bind(this),
          'style': this._feature(this.navigationBarStyle, null)
        }
      ]
    });
  },

  _setRealizedApp: function(realized) {
    this.naviframe = realized;
    this._setViewContent(realized.elements[0]);
    this.didRealizeView();
  },

  _isRealized: function() {
    return this.naviframe instanceof elm.Naviframe;
  },

  _isEqual: function(realized) {
      return ((this.naviframe) && (realized === this.naviframe));
  },

  didChangeModel: function (indexes, hint) {
    this._cachedItem = undefined;
    this.updateView(indexes, hint);
    this.evaluateViewChanges();
  },

  willInitialize: function () {},

  didInitialize: function () {},

  didRealizeView: function () {
    if (this.model)
      this.model.addController(this);
    this.updateView();
    this.evaluateViewChanges();
  },

  _evaluateToolbarChanges: function() {
    var items = this._getToolBarItems();
    var toolbar = this.viewContent.content.toolbar;

    if (toolbar.cachedItems == items)
      return;

    this.viewContent.signal_emit('bottom,toolbar', '');
    var toolbarVisible = this._feature(this.hasToolbar, items.length > 0);
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

  _createNavigationBarItem: function(item) {

    if (typeof(item) === 'string') {

      var btn;

      switch (item) {
        case 'back':
          btn = elm.Button({
            icon: this.parent.icon || 'arrow_left',
            label: this.parent._getTitle() || 'Back',
            on_click: this.popController.bind(this)
          });
          break;
        case 'sidePanel':
          btn = elm.Button({
            icon: this.split.left.icon || 'apps',
            label: this.split.left._getTitle(),
            on_click: function() { this.leftPanelVisible = true }.bind(this.split)
          });
          break;
        default:
          btn = elm.Button({
            label: item,
            on_click: this.selectedNavigationBarItem.bind(this, item)
          });
      }

      if (!btn.label)
        delete btn.label;

      if (btn.icon)
        btn.icon = elm.Icon({ image: btn.icon });
      else
        delete btn.icon;

      return btn;
    }

    if (item instanceof Controller) {

      var btn = elm.Button({});
      var title = item._getTitle();

      if (title)
        btn.label = title;

      if (item.icon)
        btn.icon = elm.Icon({ image: item.icon });

      if (item === this.parent)
        btn.on_click = this.popController.bind(this);
      else if (item === this.split.left)
        btn.on_click = function() { this.split.leftPanelVisible = true }.bind(this);
      else
        btn.on_click = this.selectedNavigationBarItem.bind(this, item);

      return btn;
    }

    if (typeof(item) === 'object' && item.hasOwnProperty('label')) {
      var props = {
        label: item.label,
        on_click: this.selectedNavigationBarItem.bind(this, item)
      };

      for (var i = 0, keys = Object.keys(item), j = keys.length; i < j; ++i)
        props[keys[i]] = item[keys[i]];

      return elm.Button(props);
    }

    return item;
  },

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

  _evaluateNavigationBarChanges: function() {
    var items = this._getNavigationBarItems();

    if (items == this.naviframe.cachedItems && title == this.naviframe.cachedTitle)
      return;

    var title = this._getTitle();
    this.naviframe_item.title = title || '';
    this.naviframe.title_visible = this._feature(this.titleVisible,
                                                title || Object.keys(items).length);

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

  _updateInterfaceElementVisibility: function() {
    if (this._feature(this.titleVisible, true))
      this.naviframe.title_visible = this._feature(this.hasNavigationBar, true);
  },

  _updateWindowProperties: function() {
    EUI.window.fullscreen = this._feature(this.isFullscreen, false);
  },

  evaluateViewChanges: function() {
    this._evaluateToolbarChanges();
    this._evaluateNavigationBarChanges();
    this._updateInterfaceElementVisibility();
    this._updateWindowProperties();
  },

  selectedNavigationBarItem: function() {}
});

Controller.prototype.__defineGetter__("searchBarVisible", function() {
  return this._cachedSearchBarVisible;
});
Controller.prototype.__defineSetter__("searchBarVisible", function(setting) {
  if (setting === this._cachedSearchBarVisible)
    return;
  this._cachedSearchBarVisible = setting;
  this._getView().signal_emit(setting ? "show,search" : "hide,search", "");
});

Controller.prototype.__defineGetter__("split", function() {
  if (this instanceof SplitController)
    return this;
  if (this.parent)
    return this.parent.split;
  return undefined;
});

Model = Class.extend({
  willInitialize: function(){
    this.controllers = [];
  },
  itemAtIndex: function(index) {
    return null;
  },
  length: function() {
    return 0;
  },
  notifyControllers: function(indexes, hint) {
    if (this.controllers.length)
      for (var i = 0; i < this.controllers.length; i++)
        this.controllers[i].didChangeModel(indexes, hint);
  },
  addController: function(controller) {
    if (this.controllers.indexOf(controller) < 0)
      this.controllers.push(controller);
  },
  removeController: function(controller){
    for (var i = 0; i < this.controllers.length; i++)
      if (this.controllers[i] == controller)
        this.controllers.splice(i, 1);
  },
  deleteItemAtIndex: function(index) {},
  updateItemAtIndex: function(index, data) {},
});

Model.prototype.__defineGetter__('selectedIndex', function() {
  return this._selectedIndex;
});
Model.prototype.__defineSetter__('selectedIndex', function(value) {
  if (typeof(value) !== 'number' || this._selectedIndex === value)
    return;
  this._selectedIndex = value;
  this.notifyControllers(value, 'select');
});

ArrayModel = Model.extend({
  init: function(array) {
    this.array = [].concat(array);
  },
  itemAtIndex: function(index) {
    return this.array[index];
  },
  length: function() {
    return this.array.length;
  },
  deleteItemAtIndex: function(index) {
    this.array.splice(index, 1);
    this.notifyControllers(index, 'delete');
  },
  updateItemAtIndex: function(index, data) {
    var item = this.array[index];
    if (typeof(data) === 'object' && typeof(item) === 'object') {
      for (var i in data)
        if (data.hasOwnProperty(i))
          item[i] = data[i];
    } else {
      this.array[index] = data;
    }
    this.notifyControllers(index, 'update');
  },
  indexOf: function(data) {
    return this.array.indexOf(data);
  },
  pushItem: function(data) {
    this.notifyControllers(this.array.push(data) - 1, 'insert');
  }
});

FilteredModel = Model.extend({
  init: function(array) {
    this.array = taffy();
    for (var i in array) {
      if(!array.hasOwnProperty(i))
        continue;
      this.array.insert({item: array[i]});
    }
  },
  itemAtIndex: function(index) {
    var item = this.array(this.filter).get()[index];
    return item && item.item;
  },
  length: function() {
    return this.array(this.filter).count();
  },
  setFilter: function(filter) {
    if (filter) this.filter = {item: filter};
    else delete this.filter;
    this.notifyControllers();
  },
  deleteItemAtIndex: function(index) {
    var item = this.array(this.filter).get()[index];
    this.array(item['___id']).remove();
    this.notifyControllers(index, 'delete');
  },
  updateItemAtIndex: function(index, data) {
    var item = this.array(this.filter).get()[index];
    this.array(item['___id']).update({item: data});
    this.notifyControllers([index]);
  },
  pushItem: function(data) {
    this.array.insert({item: data});
    this.notifyControllers();
  }
});

FileModel = Model.extend({
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
      this.notifyControllers();
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
  itemAtIndex: function(index) {
    return this.array[index];
  },
  length: function() {
    return this.array.length;
  }
});

DBModel = Model.extend({
  willInitialize: function(database) {
    this._super();
    this.entries = taffy();
    this.entries.store(database);
  },
  itemAtIndex: function (index) {
    return this.entries().get()[index];
  },
  length: function() {
    return this.entries().count();
  },
  updateItemAtIndex: function(index, values) {
    var item = this.itemAtIndex(index);
    this.entries(item['___id']).update(values);
    this.notifyControllers([index]);
  },
  insert: function(data) {
    this.entries.insert(data);
    this.notifyControllers();
  },
  indexOf: function(item, key) {
    var search = {};
    key = key || 'id';
    search[key] = item[key];
    return this.entries().get().indexOf(this.entries(search).first());
  },
  deleteItemAtIndex: function(index) {
    var item = this.itemAtIndex(index);
    this.entries(item['___id']).remove();
    this.notifyControllers();
  },
  clear: function() {
    this.entries().remove();
    this.notifyControllers();
  }
});

ActionSheet = Class.extend({
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

GenController = Controller.extend({
  willInitialize: function(_type) {
    this._groups = {};
    this._type = _type;
    this.viewDescriptor = elm.Layout({
      horizontal: false,
      file: {
        name: "eui.edj",
        group: "list"
      },
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
            this.search(this._getView().content.search.text);
          }.bind(this)
        }),
        list: _type({
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
            }
          },
          elements: {}
        })
      }
    });
  },
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
  _itemFromData: function(data) {
    if (data.model_index !== undefined)
      return this._fetchItemUsingCache(data.model_index);

    if (data.group !== undefined)
      return { text: data.group };

    return {};
  },
  didInitialize: function(_type) {
    if (this.editable) {
      var items = this._getNavigationBarItems();
      if (!items.right) {
        this.navigationBarItems = this.navigationBarItems || {};
        this.navigationBarItems.right = 'Add';
      }
      this.viewDescriptor.content.list.on_longpress = function (item) {
        item['class'] = this._getView().content.list.classes['delete'];
      }.bind(this);
    }
    this._super();
  },
  updateItemAtIndex: function(index) {
    var item = this._fetchItemUsingCache(index);
    if (!item)
      return;

    var view = this._getView().content.list;
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
      class: view.classes.default,
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
  _updateIndexes: function(indexes) {
    indexes = [].concat(indexes);

    for (var i = 0; i < indexes.length; i++)
      this.updateItemAtIndex(indexes[i]);
  },
  _updateAllIndexes: function(view, indexes) {
    this._groups = {};
    for (var i = 0, j = this.model.length(); i < j; ++i) {
      this.updateItemAtIndex(i);
    }

    while (view.elements[i] !== undefined) {
      delete view.elements[i];
      i++;
    }
  },
  updateView: function(indexes, hint) {
    this.searchBarVisible = !!this.search;

    if (hint === 'select')
      return;

    var view = this._getView().content.list;

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
});

ListController = GenController.extend({
  willInitialize: function() {
    this._super(elm.Genlist);
    this.contextMenuDirection = 'horizontal';
  },
  didInitialize: function() {
    this._super();
    var list = this.viewDescriptor.content.list;
    list.on_longpress = this.on_longpress.bind(this);
    list.on_scrolled_over_top_edge = this.on_scrolled_over_edge.bind(this, "top");
    list.on_scrolled_over_bottom_edge = this.on_scrolled_over_edge.bind(this, "bottom");
    list.on_scrolled_over_left_edge = this.on_scrolled_over_edge.bind(this, "left");
    list.on_scrolled_over_right_edge = this.on_scrolled_over_edge.bind(this, "right");
  },
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
      ctxMenu.x = this._getView().content.list.pointer.x;
      ctxMenu.y = this._getView().content.list.pointer.y;
      ctxMenu.listIndex = item.data.model_index;
      ctxMenu.show();
    }
  },
  on_scrolled_over_edge: function(edge) {
    if (typeof(this.didScrollOverEdge) === 'function')
      this.didScrollOverEdge(edge);
  }
});

GridController = GenController.extend({
  willInitialize: function() {
    this._super(elm.Gengrid);
  },
  didRealizeView: function() {
    this._getView().content.list.item_size_vertical = 128;
    this._getView().content.list.item_size_horizontal = 128;
    this._super();
  }
});

var WebController = Controller.extend({
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

  /*
   * @param url Initial WebController page.
   *
   * @brief Description of elev8 widgets that will be used on this application
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

  updateView: function() {
    var view = this._getView();

    if (!view)
      return;

    if (!this.web) {
      this.progress = view.elements.progress;
      this.web = view.elements.web;
      this.link_hover_notify = view.elements.link_hover_notify;
    }
  },

  goBack: function() {
    this.web.back();
  },

  goForward: function() {
    this.web.forward();
  },

  reload: function() {
    this.web.reload();
  },

  stop: function() {
    this.web.stop();
  },

  go: function(uri) {
    this.link_hover_notify.visible = false;
    this.progress.visible = false;
    this.web.uri = uri;
  },

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

  willPopController: function() {
    this.web.on_link_hover_in = null;
    this.web.on_link_hover_out = null;
    this.web.on_load_progress = null;
    this.link_hover_notify.visible = false;
    this.progress.visible = false;
  }
});

Container = Controller.extend({
  evaluateViewChanges: function() {},
  _getViewDescriptor: function() {
    return this.viewDescriptor;
  },
  _realizeApp: function() {
    return this._getViewDescriptor();
  },
  _setViewContent: function(view) {
    throw "It's a 'Container' and can't be pushed in a regular 'Controller'";
  },
  _setRealizedApp: function(realized) {
    this.view = realized;
    this.didRealizeView();
  },
  _getView: function() {
    return this.view;
  },
  didRealizeView: function () {
    if (this.model)
      this.model.addController(this);
    this.updateView();
  },
  promoteController: function(index) {
    this.updateView(index, 'select');
  }
});

FrameController = Container.extend({
  viewDescriptor: elm.Naviframe({
    title_visible: false,
    elements: {}
  }),
  updateView: function(index, hint) {

    var ctrl = this.model.itemAtIndex(index || 0);
    if (!ctrl) return;

    var view = this._getView();

    if (ctrl._isRealized()) {
      var elements = view.elements;
      for (var i in elements)
        if (ctrl._isEqual(elements[i].content))
          elements[i].promote();
    } else {
      var len = Object.keys(view.elements).length;
      ctrl.parent = this;
      view.elements[len] = { content: ctrl._realizeApp() };
      ctrl._setRealizedApp(view.elements[len].content);
    }
  },
});

SplitController = Container.extend({
  viewDescriptor: elm.Layout({
    expand: 'both',
    fill: 'both',
    resize: true,
    file: { name: 'eui.edj', group: 'split' },
    content: {}
  }),
  updateView: function(index, hint) {
    var view = this._getView();
    var panels = ['left', 'right'];
    var len = Math.min(this.model.length(), panels.length);

    for (var i = 0; i < len; i++) {

      var ctrl = this.model.itemAtIndex(i);
      var panel = panels[i];

      if (ctrl._isRealized() && ctrl._isEqual(view.content[panel]))
        continue;

      this[panel] = ctrl;
      ctrl.parent = this;

      var app = ctrl._realizeApp();
      delete app.resize;
      delete app.expand;
      delete app.fill;

      view.content[panel] = app;
      ctrl._setRealizedApp(view.content[panel]);
    }
  },
});

SplitController.prototype.__defineSetter__("leftPanelVisible", function(setting) {
  this._getView().signal_emit(setting ? "show,left" : "hide,left", "");
});

ToolController = Container.extend({
  viewDescriptor: elm.Toolbar({
    shrink_mode: 'expand',
    select_mode: 'always',
    elements: {}
  }),

  selectedItemAtIndex: function(index) {},

  updateView: function(index, hint) {

    var elements = this._getView().elements;
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
      real.label = ctrl._getTitle();
      real.icon = ctrl.icon;

    }.bind(this);

    /* Update controllers items */
    if (index !== undefined)
      update(index);
    else
      for (var len = this.model.length(), index = 0; index < len; index++)
        update(index);

    /* Update toolbar items */
    var items = this._feature(this.toolbarItems, []);
    if (items.length) {
      var len = this.model.length();
      elements[len] = { separator: true };
      for (var i = 0; i < items.length; i++) {
        elements[len + i] = items[i];
        elements[len + i].on_select = this.selectedToolbarItem.bind(this, elements[len + i]);
      }
    }
  }
});

TabController = Container.extend({
  viewDescriptor: elm.Layout({
    expand: 'both',
    fill: 'both',
    resize: true,
    file: { name: 'eui.edj', group: 'app' },
    content: {}
  }),

  didRealizeView: function() {

    this.toolbar = new ToolController();
    this.toolbar.model = this.model;
    this.toolbar.toolbarItems = this.toolbarItems;
    this.toolbar.selectedToolbarItem = this.selectedToolbarItem;
    this.toolbar.selectedItemAtIndex = function(index) {
      this.model.selectedIndex = index;
    };

    var view = this._getView();
    view.content.toolbar = this.toolbar._realizeApp();
    this.toolbar._setRealizedApp(view.content.toolbar);

    this.frame = new FrameController();
    this.frame.parent = this;
    this.frame.model = this.model;

    view.content.view = this.frame._realizeApp();
    this.frame._setRealizedApp(view.content.view);

    if (this.model.length())
      this.model.selectedIndex = 0;
  },

  updateView: function(index, hint) {

    var view = this._getView();
    view.signal_emit(this.tabPosition + ',toolbar', '');
    view.signal_emit((this.hasTabBar == false ? 'hide' : 'show') + ',toolbar', '');
  },
});

ImageController = Controller.extend({
  viewDescriptor: elm.Photocam({
    expand: 'both',
    fill: 'both',
    zoom_mode: "auto-fill",
    zoom: 5.0
  }),
  navigationBarStyle: 'overlap',
  didRealizeView: function() {
    this._getView().on_click = function() {
      if (this.didClickView)
        this.didClickView();
    }.bind(this);
    this._super();
  },
  length: function () {
    return this.model.length();
  },
  setImage: function(index) {
    if ((index < 0) || (index >= this.length()))
      return;
    this.index = index;
    this.didChangeModel();
  },
  updateView: function() {
    this._getView().file = this.model.itemAtIndex(this.index).path;
  },
});

VideoController = Controller.extend({
  viewDescriptor: elm.Box({
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
  }),
  playerBar : ['Play', 'Pause'],
  length: function () {
    return this.model.length();
  },
  setVideo: function(index) {
    if ((index < 0) || (index >= this.length()))
      return;
    this.index = index;
    this.didChangeModel();
  },
  itemAtIndex : function(index){
    return this.model.itemAtIndex(this.index);
  },
  updateView: function() {
    var elements = this._getView().elements;
    if (!elements.controllers.video)
      elements.controllers.video = elements.video;

    elements.video.file = this.itemAtIndex(this.index);
  },
});

TableController = Controller.extend({
  viewDescriptor: elm.Table({
    expand: 'both',
    fill: 'both',
    elements: {}
  }),
  updateView: function(index, hint) {

    var elements = this._getView().elements;
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
          real.editable = this.editable && item.editable;
        } else {
          real.editable = false;
        }
      }
    }
  },
  getValues: function() {
    var values = {};
    var elements = this._getView().elements;
    for (var key in elements) {
      var real = elements[key].element;
      if (real && real.field)
        values[key] = real.getValue();
    }
    return values;
  }
});

TableController.prototype.__defineGetter__("index", function() { return this._index });
TableController.prototype.__defineSetter__("index", function(value) {
  if (!this.model) throw "Model must be defined before index";
  if (value >= 0 && value < this.model.length())
    this._index = value;
});

FormController = TableController.extend({
  navigationBarItems: function() {
    return this.editable && {
      left: 'Cancel',
      right: (this.index === undefined) ? 'Add' : 'Save'
    };
  },
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

RoutingSingleton = Class.extend({
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

  possibleActions: function() {
    return Object.keys(this.database);
  },

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

  edit: function(mime_type, file) {
    return this.performAction('edit', mime_type, file);
  },

  share: function(mime_type, file) {
    return this.performAction('share', mime_type, file);
  },

  view: function(mime_type, file) {
    return this.performAction('view', mime_type, file);
  },

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
exports.app = function(app) {

  if (!(app instanceof Controller))
    throw "First argument must be a Controller instance.";

  EUI.window = elm.realise(elm.Window({
    width: 320,
    height: 480,
    title:  app._feature(app.title, 'EasyUI Application'),
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

  EUI.window.elements.app = app._realizeApp();
  app._setRealizedApp(EUI.window.elements.app);
};

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

exports.applyController = function(controller) {
  var model = controller._feature(controller.model, null);
  if (model)
    model.notifyControllers();
};

exports.popController = function() {
  var oldController = EUI.controllers.pop();
  if (EUI.controllers.length == 0)
    elm.quit();

  var controller = EUI.controllers[EUI.controllers.length - 1];
  EUI.applyController(controller);
  EUI.window.elements.naviframe.pop(oldController.evasObject());
};

exports.DBModel = DBModel;
exports.FileModel = FileModel;
exports.FilteredModel = FilteredModel;
exports.ArrayModel = ArrayModel;
exports.Routing = Routing;

exports.Model = function(proto) {
  return Model.extend(proto);
}

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
}

exports.SplitController = function(proto) {
  return SplitController.extend(proto);
};

exports.WebController = function(proto) {
  return WebController.extend(proto);
}

exports.VideoController = function(proto) {
  return VideoController.extend(proto);
}

exports.widgets = {};

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

exports.widgets.Button = wrapElm(elm.Button, {
  event_map: {
    on_click: 'didClickOnElement',
    on_press: 'didPressOnElement',
    on_release: 'didReleaseElement'
  },
  setValue: function (value) { if (this.text != value) this.text = value },
  getValue: function() { return this.text }
});

exports.widgets.Label = wrapElm(elm.Label, {
  setValue: function (value) { this.file = value; }
});

exports.widgets.Entry = wrapElm(elm.Entry, {
  event_map: {
    on_activate: 'didActivate',
    on_change: 'didChangeEntry'
  },
  setValue: function (value) {
    value = (value !== undefined) ? value : '';
    if (this.text != value)
      this.text = value;
  },
  getValue: function() { return this.markup_to_utf8(this.text) }
});

exports.widgets.Photocam = wrapElm(elm.Photocam, {
  setValue: function(value) { this.file = value },
  getValue: function() { return this.file }
});

exports.widgets.Web = wrapElm(elm.Web, {
  type: 'Web',
  event_map: {
    on_load_progress: 'didLoadProgress',
    on_ready: 'didReady',
    on_title_change: 'didTitleChange',
    on_uri_change: 'didUriChange',
  },
});

exports.widgets.Icon = wrapElm(elm.Icon, {
  setValue: function (value) { if (this.file != value) this.file = value },
  getValue: function() { return this.file }
});

exports.widgets.Check = wrapElm(elm.Check, {
  setValue: function(value) { this.state = !!value },
  getValue: function() { return this.state }
});

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
