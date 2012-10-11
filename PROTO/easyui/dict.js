EUI = require('eui');
dump = require('dump').dump;

localStorage.database = "dict.eet";
elm.addThemeOverlay("./themes/dict.edj");
imagedir = 'themes/dict/images/';

var bookmarks = new EUI.DBModel('bookmarks');
var history = new EUI.DBModel('history');

DictModel = EUI.Model({
  indexes: [],
  entries: [],
  length: function() {
    return this.entries.length;
  },
  itemAtIndex: function(index) {
    return this.entries[index];
  },
  setFilter: function(terms) {

    if (this.timeout)
      clearTimeout(this.timeout);

    this.timeout = setTimeout(function() {
      this.timeout = clearTimeout(this.timeout);

      if (!terms.length) {
        this.entries = [];
        this.notifyControllers();
      }

      terms = terms.toUpperCase();
      var firstLetter = terms[0];
      if (!this.indexes[firstLetter]) {
        var index = localStorage.getItem("i" + firstLetter);
        if (!index)
          return [];
        this.indexes[firstLetter] = JSON.parse(index);
      }

      var tmp = this.indexes[firstLetter];
      for (var i = 1; tmp && i < terms.length; i++)
        tmp = tmp[terms[i]];

      var linearize = function(tree, current_word) {
        var output = [];
        for (var key in tree) {
          if (!tree.hasOwnProperty(key))
            continue;
          if (key == '_')
            output.push({text: (terms + current_word).toLowerCase(), id: tree[key]})
          else
            output = output.concat(linearize(tree[key], current_word + key));
        }
        return output;
      }

      tmp = linearize(tmp, '');
      tmp.sort(function(a, b) {
        if (a.text == b.text)
          return 0;
        if (a.text < b.text)
          return -1;
        return 1;
      });

      this.entries = tmp;
      this.notifyControllers();

    }.bind(this), 200);
  }
});

var dictModel = new DictModel();

ListItems = EUI.ListController({
  navigationBarStyle: 'dict',
  itemAtIndex: function(index) {
    var item = this.model.itemAtIndex(index);
    return item && {text: item.text};
  },
  selectedItemAtIndex: function(index) {
    var item = this.model.itemAtIndex(index);
    this.pushController(new Definition(item));
  },
});

Bookmarks = ListItems.extend({
  title: 'Bookmarks',
  icon: 'bookmarks',
  model: bookmarks,
});

History = ListItems.extend({
  title: 'History',
  icon: 'history',
  model: history,
  navigationBarItems: { right: 'Clear' },
  selectedNavigationBarItem: function (item) {
    if (item === 'Clear')
      history.clear();
  }
});

Search = ListItems.extend({
  title: 'Search',
  icon: 'search',
  model: dictModel,
  hasNavigationBar: false,
  searchBarItems: {
    left: EUI.widgets.Button({
      icon: EUI.widgets.Icon({ file: imagedir + 'search.png'})
    }),
    right: EUI.widgets.Button({
      icon: EUI.widgets.Icon({ file: imagedir + 'del.png'})
    })
  },
  search: function(text) {
    this.model.setFilter(text);
  }
});

Definition = EUI.TableController({
  init: function (item) {

    var buffer = '';
    for (var def = 0; ; def++) {
      var pronounciation = localStorage.getItem(item.id + "p" + def);
      if (!pronounciation)
        break;
      var definition = localStorage.getItem(item.id + "d" + def);
      if (!definition)
        break;
      buffer += "<b>" + pronounciation + "</b><br>" + definition + "<br><br>";
    }

    this.item = item;
    this.title = item.text;
    this.model = new EUI.ArrayModel([{entry: buffer}]);
    this.index = 0;
    this.bookmark = (bookmarks.indexOf(item) > -1);
  },
  fields: [[EUI.widgets.Entry({text: '', scrollable: true, field: 'entry', style: 'no_border'})]],
  navigationBarStyle: 'dict',
  navigationBarItems: function() {
    return {
      right: EUI.widgets.Check({
        ctrl: this,
        state: this.bookmark,
        on_change: function() { this.ctrl.bookmark = this.state }
      }),
      left: EUI.widgets.Button({
        icon: EUI.widgets.Icon({ file: imagedir + 'back.png' }),
        on_click: function() { this.popController() }.bind(this)
      })
    };
  },
  willPopController: function() {

    /* Update History */
    if (history.indexOf(this.item) < 0)
      history.insert(this.item);

    /* Update Bookmarks */
    var index = bookmarks.indexOf(this.item);
    if (this.bookmark && index < 0)
      bookmarks.insert(this.item);
    else if (!this.bookmark && index > -1)
      bookmarks.deleteItemAtIndex(index);
  }
});

Dict = EUI.TabController({
  title: 'Dictionary',
  model: new EUI.ArrayModel([new Search(), new Bookmarks(), new History()]),
  tabPosition: 'bottom',
  hasNavigationBar: false,
});

EUI.app(new Dict());
