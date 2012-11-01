/* Load the Elev8 modules to be used */
var EUI = require('eui');
var XMLHttpRequest = require('http').XMLHttpRequest;

/* Extend the base model to work with Infinigag JSON API */
InfinigagModel = EUI.Model({
  items: [],
  url: 'infinigag.com/api.json?page=',

  /* Init method is the constructor */
  init: function() {
    this.request = new XMLHttpRequest();
    this.request.onreadystatechange = function() {

      var resp = JSON.parse(this.request.responseText);
      this.next = resp['attributes']['next'];

      /* Fill the model content with the info received */
      this.items = this.items.concat(resp['images']);

      /* Notify the controller about model changes */
      this.notifyListeners();
    }.bind(this);
    this.refresh();
  },

  /* Return the item at index or undefined if it doesn't exist */
  itemAtIndex: function(index) {
    var item = this.items[index];

    /* If no file defined or requested */
    if (!item.file && !item.request) {

      /* Ask for image file */
      item.request = new XMLHttpRequest();
      item.request.onreadystatechange = function(index) {
        var item = this.items[index];
        if (item == undefined) return;
        item.file = item.request.responseText;
        delete item.request;

        /* Notify the controller about item updated at index */
        this.notifyListeners(index);
      }.bind(this, index);

      item.request.open("GET", item.image.big);
      item.request.send("");
    }
    return item;
  },

  /* Get new information from Infinigag */
  refresh: function() {
    delete this.next;
    this.items = [];
    this.more();
  },

  more: function() {
    this.request.open("GET", this.url + this.next);
    this.request.send("");
  },

  /* Return the model's length */
  length: function() { return this.items.length; },
});

/* Extend the TableController */
Infinigag = EUI.TableController({

  /* Create a model and connect it to the controller */
  init: function(model, index) {
    this.model = model;
    this.index = index;
  },

  /* TableController uses a bidimensional array to posit its widgets */
  fields: [[EUI.widgets.Photocam({zoom_mode: 'auto-fit', field: 'file'})]],

  /* The content of toolbar */
  toolbarItems: [
    {label: 'Prev', icon: 'arrow_left'},
    {label: 'Next', icon: 'arrow_right'},
  ],

  /* Function to be called when a toolbar item be selected */
  selectedToolbarItem: function(item) {
    switch (item.label) {
      case 'Next': this.index++; break;
      case 'Prev': this.index--; break;
    }
    this.model.notifyListeners(this.index);
  },

  /* Use the item title */
  title: function() {
    var item = this.model.itemAtIndex(this.index);
    return item && item.title || "Infinigag";
  }
});

InfinigagList = EUI.ListController({
  model: new InfinigagModel(),
  itemAtIndex: function(index) {
    var item = this.model.itemAtIndex(index);
    return {text: item.title, icon: item.file };
  },
  selectedItemAtIndex: function(index) {
    this.pushController(new Infinigag(this.model, index));
  },
  toolbarItems: [
    {label: 'More', icon: 'apps'},
    {label: 'Refresh', icon: 'refresh'},
    {label: 'Exit', icon: 'close'},
  ],
  selectedToolbarItem: function(item) {
    switch (item.label) {
      case 'More': this.model.more(); break;
      case 'Refresh': this.model.refresh(); break;
      case 'Exit': elm.exit(); break;
    }
  }
});

/* Start the application */
EUI.app(new InfinigagList());
