var EUI = require("eui");

var MyList = EUI.ListController({
  model: new EUI.FilteredModel(['Apple', 'Banana', 'Orange', 'Watermelon', 'Melon']),
  title: "Filter",
  itemAtIndex: function(index) {
    var item = this.model.itemAtIndex(index);
    return {text: item};
  },
  search: function(text) {
    if (text == '')
      this.model.setFilter(null);
    else
      this.model.setFilter({likenocase: text});
  },
  toolbarItems: ['Apple', 'Melon', 'All'],
  selectedToolbarItem: function (item) {
    if (item == 'All')
      this.model.setFilter(null);
    else
      this.model.setFilter({likenocase: item});
  }
});

EUI.app(new MyList());
