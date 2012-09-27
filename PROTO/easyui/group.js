EUI = require('eui');

var counter = 0;

grouped = function(text, group) {
  return {
    text: text,
    group: group
  };
};

C = EUI.ListController({
  title: 'Group Test',
  model: new EUI.ArrayModel([
    grouped('News Feed', 'Favorites'),
    grouped('Messages', 'Favorites'),
    grouped('Nearby', 'Favorites'),
    grouped('Events', 'Favorites'),
    grouped('Friends', 'Favorites'),

    grouped('EasyUI', 'Pages'),
    grouped('Elev8', 'Pages'),

    grouped('Enlightenment', 'Groups'),

    grouped('Close Friends', 'Friends'),
    grouped('Family', 'Friends'),

    grouped('App Center', 'Apps'),
    grouped('Chat', 'Apps'),
    grouped('Find Friends', 'Apps'),
    grouped('Photos', 'Apps'),

    grouped('Edit Favorites', 'Misc'),
    grouped('Account', 'Misc'),
    grouped('Help', 'Misc'),
  ]),
  selectedItemAtIndex: function(index) {
    this.selIdx=index;
  },
  itemAtIndex: function(index) {
    var item = this.model.itemAtIndex(index);
    return {
      text: index + "; "  + item.text,
      group: item.group
    }
  },
  toolbarItems: [
    'Upd', 'DelSel', 'AddNew', 'AddFav'
  ],
  selectedToolbarItem: function(item) {
    if (item === 'Upd') {
      this.updateView();
    } else if (item === 'DelSel') {
      if (this.selIdx === undefined)
        print('select an item and try again');
      else {
        this.model.deleteItemAtIndex(this.selIdx);
        this.selIdx = undefined;
      }
    } else if (item === 'AddNew') {
      this.model.pushItem(grouped(counter++, 'New Group'))
    } else if (item === 'AddFav') {
      this.model.pushItem(grouped(counter++, 'Favorites'))
    } else {
      print('Unknown toolbar item clicked', item);
    }
  }
});

EUI.app(new C);
