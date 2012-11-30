var EUI = require("eui");

Fruits = new EUI.ArrayModel([
  {type: 'Citrus', fruit:'orange'},
  {type: 'Citrus', fruit: 'limon'},
  {type: 'Dried', fruit: 'apricot'},
  {type: 'Dried', fruit: 'prune'}
]);

var MyList = EUI.ListController({
  model: new EUI.FilterModel(
    Fruits,
    function (item) {
      if (item.type === 'Citrus')
        return item;
    }
  ),
  itemAtIndex: function(index) {
    var item = this.model.itemAtIndex(index);
    return {text: item.fruit};
  },
});

EUI.app(new MyList());
