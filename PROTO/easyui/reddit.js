EUI = require('eui');
ajax = require('ajax');
Property = require('class').Property;

const reddit_url = 'http://www.reddit.com/';

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
   * Refresh model after finishing XML file download
   * @event
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
    return this._array[index];
  }
});

/** @extends EUI.ListController */
RSSList = EUI.ListController({
  /** @type {String} */
  style: 'double_label',
  /** @type {String} */
  group_name: 'RSS',
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
      text_sub: item.description
    };
  },
  /** @inheritdoc */
  selectedItemAtIndex: function(index) {
    var item = this.model.itemAtIndex(index);
    this.pushController(new WebBrowser(item.link));
  },
  /** @type {Object} */
  navigationBarItems: { left: 'sidePanel' }
});

/** @extends EUI.Model */
RedditModel = EUI.Model({
  /**
   * @param {String} endpoint
   * @event
   */
  init: function(endpoint) {
    this.items = [];
    this.url = reddit_url + endpoint;

    this.refresh();
  },
  /**
   * Refresh model after finishing HTML file download
   */
  refresh: function() {
    ajax.ajax(this.url, {
      onSuccess: function(request) {
        this.items = JSON.parse(request.responseText).data.children;
        this.notifyListeners();
      }.bind(this)
    });
  },
  /** @inheritdoc */
  length: function() {
    return this.items.length;
  },
  /** @inheritdoc */
  itemAtIndex: function(index) {
    return this.items[index].data;
  }
});

/** @extends EUI.WebController */
WebBrowser = EUI.WebController({
  /** @type {Array} */
  navigationBarItems: [undefined, 'Articles'],
});

/** @extends EUI.ListController */
RedditList = EUI.ListController({
  /** @type {String} */
  style: 'double_label',
  /** @type {String} */
  group_name: 'Subreddits',
  /**
   * @param {String} endpoint
   * @param {String} title
   * @param {String} icon
   * @event
   */
  init: function(endpoint, title, icon) {
    this.model = new RedditModel(endpoint);
    this.title = title;
    this.icon = icon;
  },
  /** @inheritdoc */
  itemAtIndex: function(index) {
    var item = this.model.itemAtIndex(index);
    return {
      text: item.title,
      text_sub: 'Posted by ' + item.author + ' in ' + item.subreddit
    };
  },
  /** @inheritdoc */
  selectedItemAtIndex: function(index) {
    var url = this.model.itemAtIndex(index).permalink;
    if (url)
      url = reddit_url + url + '.compact';
    else
      url = item.url;
    this.pushController(new WebBrowser(url));
  },
  /** @type {Object} */
  navigationBarItems: { left: 'sidePanel', right: 'Refresh' },
  /** @inheritdoc */
  selectedNavigationBarItem: function(item) {
    if (item == 'Refresh')
      this.model.refresh();
  }
});

/** @extends EUI.Model */
SearchModel = EUI.Model({
  /** @inheritdoc */
  init: function() {
    this.array = [];
  },
  /** @inheritdoc */
  itemAtIndex: function(index) {
    if (index < this.array.length)
      return this.array[index];
  },
  /** @inheritdoc */
  length: function() {
    return this.array.length;
  },
  /** @type {String} */
  filter: new Property ({
    set: function(terms) {
      var query = terms.split(' ');
      this.array = [];
      ajax.get(reddit_url + '/r/all/search.json?q=', query, function(request) {
        var subr = {};
        var temp = JSON.parse(request.responseText).data.children;
        for (var i in temp)
          subr[temp[i].data.subreddit] = 1;
        this.array = Object.keys(subr);
        this.notifyListeners();
      }.bind(this));
    }
  }),
});

/** @extends EUI.ListController */
RedditSearchController = EUI.ListController({
  /** @type {String} */
  title: 'Search',
  /** @type {SearchModel} */
  model: new SearchModel(),
  /** @inheritdoc */
  itemAtIndex: function(index) {
    var item = this.model.itemAtIndex(index);
    return item && {text: item};
  },
  /** @inheritdoc */
  selectedItemAtIndex: function(index) {
    subreddit = this.model.itemAtIndex(index);
    reddit_all_items.pushItem(new RedditList('r/'+subreddit+'/top.json', subreddit, ''));
  },
  /**
   * @param {String} text
   */
  search: function(text) {
    this.model.filter = text;
  }
});

/** @extends EUI.ArrayModel */
reddit_all_items = new EUI.ArrayModel([
  new RedditList('top.json', 'Home', 'go-home'),
  new RedditList('r/programming/top.json', 'Proggit', 'applications-development'),
  new RedditList('r/iama/top.json', 'IAmA', 'dialog-question'),
  new RedditList('r/funny/top.json', 'Funny', 'face-laugh'),
  new RSSList('http://feeds.bbci.co.uk/news/uk/rss.xml', 'BBC')
]);

/** @extends EUI.FilterModel */
FilterRedditApps = new EUI.FilterModel(
  reddit_all_items,
  function (item) {
    if (item.group_name === 'Subreddits')
      return item;
  }
);

/** @extends EUI.ListController */
Subscriptions = EUI.ListController({
  /** @type {String} */
  group_name: 'Management',
  /** @type {String} */
  title: 'Subscriptions',
  /** @type {EUI.FilterModel} */
  model: FilterRedditApps,
  /** @type {Object} */
  navigationBarItems: { left: 'sidePanel', right: 'Add' },
  /** @inheritdoc */
  selectedNavigationBarItem: function(item) {
    if (item === 'Add')
      this.pushController(new RedditSearchController());
  },
  /** @inheritdoc */
  itemAtIndex: function(index) {
    return {
      text: this.model.itemAtIndex(index).title
    };
  },
});

reddit_all_items.pushItem(new Subscriptions());

/** @extends EUI.ListController */
RedditAppSwitch = new EUI.ListController({
  /** @type {EUI.ArrayModel} */
  model: reddit_all_items,
  /** @inheritdoc */
  itemAtIndex: function(index) {
    var item = this.model.itemAtIndex(index);
    return {
      text: item.title,
      icon: item.icon,
      group: item.group_name
    };
  },
  /** @inheritdoc */
  selectedItemAtIndex: function(index) {
    this.model.selectedIndex = index;
    this.split.leftPanelVisible = false;
  }
});

/** @extends EUI.FrameController */
RedditAppTabs = new EUI.FrameController({
  /** @type {EUI.ArrayModel} */
  model: reddit_all_items,
});

/** @extends EUI.SplitController */
Reddit = EUI.SplitController({
  /** @type {EUI.ArrayModel} */
  model: new EUI.ArrayModel([
    new RedditAppSwitch(),
    new RedditAppTabs()
  ])
});

EUI.app(new Reddit());
