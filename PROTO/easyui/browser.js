var EUI = require('eui');

WebPageController = EUI.WebController({
  url: 'http://www.enlightenment.org',
  titleVisible: false,
  didChangeTitle: function(title) {
    this.title = title;
    if (title)
      this.parent.updateView();
  }
});

BrowserController = EUI.TabController({
  model: new EUI.ArrayModel([new WebPageController()]),
  toolbarItems: [{tag: 'new', label: '+'}],
  selectedToolbarItem: function(item) {
    switch (item.tag) {
    case 'new':
      this.model.pushItem(new WebPageController());
      break;
    }
  }
});

EUI.app(new BrowserController());
