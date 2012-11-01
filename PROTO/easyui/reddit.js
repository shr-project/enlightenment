EUI = require('eui');
ajax = require('ajax');
Property = require('class').Property;

const reddit_url = 'http://www.reddit.com/';

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
    return this._array[index];
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
      text_sub: item.description
    };
  },
  selectedItemAtIndex: function(index) {
    var item = this.model.itemAtIndex(index);
    this.pushController(new WebBrowser(item.link));
  },
  navigationBarItems: { left: 'sidePanel' }
});

RedditModel = EUI.Model({
  init: function(endpoint) {
    this.items = [];
    this.url = reddit_url + endpoint;

    this.refresh();
  },
  refresh: function() {
    ajax.ajax(this.url, {
      onSuccess: function(request) {
        this.items = JSON.parse(request.responseText).data.children;
        this.notifyControllers();
      }.bind(this)
    });
  },
  length: function() {
    return this.items.length;
  },
  itemAtIndex: function(index) {
    return this.items[index].data;
  }
});

WebBrowser = EUI.WebController({
  navigationBarItems: [undefined, 'Articles'],
});

RedditList = EUI.ListController({
  style: 'double_label',
  init: function(endpoint, title, icon) {
    this.model = new RedditModel(endpoint);
    this.title = title;
    this.icon = icon;
  },
  itemAtIndex: function(index) {
    var item = this.model.itemAtIndex(index);
    return {
      text: item.title,
      text_sub: 'Posted by ' + item.author + ' in ' + item.subreddit
    };
  },
  selectedItemAtIndex: function(index) {
    var url = this.model.itemAtIndex(index).permalink;
    if (url)
      url = reddit_url + url + '.compact';
    else
      url = item.url;
    this.pushController(new WebBrowser(url));
  },
  navigationBarItems: { left: 'sidePanel', right: 'Refresh' },
  selectedNavigationBarItem: function(item) {
    if (item == 'Refresh')
      this.model.refresh();
  }
});

SearchModel = EUI.Model({
  init: function() {
    this.array = [];
  },
  itemAtIndex: function(index) {
    if (index < this.array.length)
      return this.array[index];
  },
  length: function() {
    return this.array.length;
  },
  filter: new Property ({
    set: function(terms) {
      var query = terms.split(' ');
      this.array = [];
      ajax.get(reddit_url + '/r/all/search.json', {q: query}, function(request) {
        var subr = {};
        var temp = JSON.parse(request.responseText).data.children;
        for (var i in temp)
          subr[temp[i].data.subreddit] = 1;
        this.array = Object.keys(subr);
        this.notifyControllers();
      }.bind(this));
    }
  }),
});

RedditSearchController = EUI.ListController({
  title: 'Search',
  model: new SearchModel(),
  itemAtIndex: function(index) {
    var item = this.model.itemAtIndex(index);
    return item && {text: item};
  },
  selectedItemAtIndex: function(index) {
    this.parent.onSubredditSelected(this.model.itemAtIndex(index));
  },
  search: function(text) {
    this.model.filter = text;
  }
});

RedditApps = new EUI.ArrayModel([
  new RedditList('top.json', 'Home', 'go-home'),
  new RedditList('r/programming/top.json', 'Proggit', 'applications-development'),
  new RedditList('r/iama/top.json', 'IAmA', 'dialog-question'),
  new RedditList('r/funny/top.json', 'Funny', 'face-laugh'),
  new RSSList('http://feeds.bbci.co.uk/news/uk/rss.xml', 'BBC')
]);

RedditAppSwitch = new EUI.ListController({
  model: RedditApps,
  itemAtIndex: function(index) {
    var item = this.model.itemAtIndex(index);
    return {
      text: item.title,
      icon: item.icon
    };
  },
  selectedItemAtIndex: function(index) {
    this.model.selectedIndex = index;
    this.split.leftPanelVisible = false;
  }
});

RedditAppTabs = new EUI.FrameController({
  model: RedditApps,
});

Reddit = EUI.SplitController({
  model: new EUI.ArrayModel([
    new RedditAppSwitch(),
    new RedditAppTabs()
  ])
});

EUI.app(new Reddit());
