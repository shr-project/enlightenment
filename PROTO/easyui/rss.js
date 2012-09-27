EUI = require('eui');
ajax = require('ajax');

const URL = 'http://feeds.bbci.co.uk/news/uk/rss.xml';

WebBrowser = EUI.WebController({
  init: function(url) {
    this.url = url;
  }
})

RSSModel = EUI.Model({
  init: function(url, path) {
    this.url = this.url || url;
    this._array = [];
    this.refresh();
  },
  refresh: function() {
    ajax.ajax(this.url, {
      onSuccess: function(request) {
        if (!request.responseXML)
          return;

        this._array = request.responseXML.rss.channel.item;
        this.notifyControllers();
      }.bind(this)
    });
  },
  length: function() {
    return this._array.length;
  },
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
          this.notifyControllers(index);
        }.bind(this)
      });
    }

    return item;
  }
});

RSSList = EUI.ListController({
  style: 'double_label',
  init: function(url, title) {
    this.title = title;
    this.model = new RSSModel(url);
  },
  itemAtIndex: function(index) {
    var item = this.model.itemAtIndex(index);

    return {
      text: item.title,
      text_sub: item.description,
      icon: item.icon
    };
  },
  selectedItemAtIndex: function(index) {
    var item = this.model.itemAtIndex(index);
    this.pushController(new WebBrowser(item.link));
  }
});

EUI.app(new RSSList(URL, 'BBC'));