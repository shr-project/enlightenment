var EUI = require('eui');
var path = './videos';
var patterns = ['*.mp*'];

VideoShareSheet = EUI.ActionSheet({
  title: 'Share Video',
  model: [
    'Email Video',
    'Send to Youtube',
    'Send via Bluetooth',
  ],
  hasCancelAction: true,
  selectedItemAtIndex: function(index) {
    switch (index) {
    case 0: break;    // Email Video
    case 1: break;    // Send to Youtube
    case 2: break;    // Send via Bluetooth
    }
  }
});

VideoController = EUI.VideoController({
  extractPathsFromFileModel: function(model){
    array = [];
    for (var i = 0, j = model.length; i < j; i++){
      if (model.itemAtIndex(i).isFile)
        array.push(model.itemAtIndex(i).path);
    }

    return array;
  },
  init: function(model, index) {
    this.title = model.itemAtIndex(index).name;
    this.model = model;
    this.index = index;
  },
  toolbarItems: function() {

    var items = ['share'];

    if (this.length > 1) {
      items.push('left');
      items.push('right');
    }
    return items;
  },
  selectedToolbarItem: function(item) {
    switch (item) {
    case 'share':
      this.pushController(new VideoShareSheet);
      break;
    case 'left':
      this.setVideo(this.index - 1);
      break;
    case 'right':
      this.setVideo(this.index + 1);
      break;
    }
  },
  itemAtIndex: function(index) {
    return this.model.itemAtIndex(index).path;
  }
});

VideoCollectionShareSheet = EUI.ActionSheet({
  title: 'Share Collection',
  model: [
      'Email Collection',
      'Send via Bluetooth'
  ],

  hasCancelAction: true,
  selectedItemAtIndex: function(index) {
    switch (index) {
    case 0: break;    /* Email Collection */
    case 1: break;    /* Send via Bluetooth */
    }
  }
});

VideoCollectionController = EUI.GridController({
  init: function(item, patterns) {
    this.title = item.name;
    this.model = new EUI.FileModel(item.path, patterns);
    print(item.path, this.model.length);
  },

  toolbarItems: [ "share" ],
  selectedToolbarItem: function(item) {
    switch (item) {
    case 'share':
      this.pushController(new VideoCollectionShareSheet());
      break;
    }
  },
  itemAtIndex: function(index) {
    var item = this.model.itemAtIndex(index);
    print(item.name);
    return {
      text: item.name,
      //TODO show an icon
      icon: null
    };
  },
  selectedItemAtIndex: function(index) {
    this.pushController(new VideoController(this.model, index));
  }
});

VideoListController = EUI.ListController({
  patterns: patterns,
  model: new EUI.FileModel(path, this.patterns),
  title: 'Video Collections',
  selectedItemAtIndex: function(index) {
    var item = this.model.itemAtIndex(index);

    if (!item.isFile)
      this.pushController(new VideoCollectionController(item, this.patterns));
  },
  itemAtIndex: function(index) {

    var item = this.model.itemAtIndex(index);
    return {
      //TODO show an icon
      icon: null,
      text: item.name
    };
  }
});

EUI.app(new VideoListController());
