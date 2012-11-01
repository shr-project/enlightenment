EUI = require('eui');
ajax = require('ajax');

const URL = 'http://feeds.bbci.co.uk/news/uk/rss.xml';

/** @extends EUI.WebController */
WebBrowser = EUI.WebController({
  /**
   * @param {String} url
   * @event
   */
  init: function(url) {
    this.url = url;
  }
})

/** @extends EUI.Model */
RSSModel = EUI.Model({
  /**
   * @param {String} url
   * @param {String} path
   * @event
   */
  init: function(url, path) {
    this.url = this.url || url;
    this._array = [];
    this.refresh();
  },
  /**
   * Refresh model after finishing XML file download.
   */
  refresh: function() {
    ajax.ajax(this.url, {
      onSuccess: function(request) {
        if (!request.responseXML)
          return;

        this._array = request.responseXML.rss.channel.item;
        this.notifyListeners();
      }.bind(this)
    });
  },
  /** @inheritdoc */
  length: function() {
    return this._array.length;
  },
  /** @inheritdoc */
  itemAtIndex: function(index) {
    var item = this._array[index];

    if (!item.hasOwnProperty('icon') && !item.request) {
      var media = item['media:thumbnail'];
      var url = undefined;

      if (media.hasOwnProperty('length'))
        url = media[0].attributes.url;
      else
        url = media.attributes.url;

      item.request = ajax.ajax(url, {
        blockUI: true,
        onSuccess: function(request) {
          item.icon = request.responseText;
          delete item.request;
          this.notifyListeners(index);
        }.bind(this)
      });
    }

    return item;
  }
});

/** @extends EUI.ListController */
RSSList = EUI.ListController({
  /** @type {String} */
  style: 'double_label',
  /**
   * @param {String} url
   * @param {String} title
   * @event
   */
  init: function(url, title) {
    this.title = title;
    this.model = new RSSModel(url);
  },
  /** @inheritdoc */
  itemAtIndex: function(index) {
    var item = this.model.itemAtIndex(index);

    return {
      text: item.title,
      text_sub: item.description,
      icon: item.icon
    };
  },
  /** @inheritdoc */
  selectedItemAtIndex: function(index) {
    var item = this.model.itemAtIndex(index);
    this.pushController(new WebBrowser(item.link));
  }
});

EUI.app(new RSSList(URL, 'BBC'));