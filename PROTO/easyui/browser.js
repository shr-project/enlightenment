var EUI = require('eui');

/** @extends EUI.WebController */
WebPageController = EUI.WebController({
  /** @type {String} */
  url: 'http://www.enlightenment.org',
  /** @type {Boolean} */
  titleVisible: false,
  /**
   * @param {String} title
   * @event
   */
  didChangeTitle: function(title) {
    var index = webPagesModel.indexOf(this);
    webPagesModel.updateItemAtIndex(index, {title: title});
  }
});

webPagesModel = new EUI.ArrayModel([new WebPageController()]);

/** @extends EUI.TabController */
BrowserController = EUI.TabController({
  /** @type {EUI.ArrayModel} */
  model: webPagesModel,
  /** @type {Array} */
  toolbarItems: [{tag: 'new', label: '+'}],
  /** @inheritdoc */
  selectedToolbarItem: function(item) {
    switch (item.tag) {
    case 'new':
      this.model.pushItem(new WebPageController());
      break;
    }
  }
});

EUI.app(new BrowserController());
