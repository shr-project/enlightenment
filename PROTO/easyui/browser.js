var EUI = require('eui');

WebPageController = EUI.WebController({
  url: 'http://www.enlightenment.org',
  titleVisible: false,
  didChangeTitle: function(title) {
    var index = webPagesModel.indexOf(this);
    webPagesModel.updateItemAtIndex(index, {title: title});
  }
});

webPagesModel = new EUI.ArrayModel([new WebPageController()]);

BrowserController = EUI.TabController({
  model: webPagesModel,
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
