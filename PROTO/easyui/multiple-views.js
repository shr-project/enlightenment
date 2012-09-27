EUI = require('eui');

m = new EUI.ArrayModel(['a', 'b', 'c', 'd', 'e']);

C = EUI.ListController({
  model: m,
  itemAtIndex: function(index) {
    return {
      text: this.model.itemAtIndex(index)
    };
  },
  toolbarItems: [ 'del(0)', 'del(1)', 'update(2)', 'newview' ],
  selectedToolbarItem: function(item) {
    if (item == 'del(0)') {
      if (this.model.length() > 0)
        this.model.deleteItemAtIndex(0);
    } else if (item == 'del(1)') {
      if (this.model.length() > 1)
        this.model.deleteItemAtIndex(1);
    } else if (item == 'update(2)') {
      if (this.model.length() > 2)
        this.model.updateItemAtIndex(2, '--- UPDATED ---');
    } else if (item == 'newview') {
      this.pushController(new C);
    }
  }
});

EUI.app(new C);
