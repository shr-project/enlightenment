var EUI = require('eui');
var cnt = 0;
var icons = [
  "Home",
  "Close",
  "Apps",
  "Arrow_up",
  "Arrow_down",
  "Arrow_left",
  "Arrow_right",
  "Chat",
  "Clock",
  "Delete",
  "Edit",
  "Refresh",
  "Folder",
  "File",
];

Controller = EUI.TableController({
  init: function(title, image) {
    this.icon = image;
    this.title = title
    this.fields = [[EUI.widgets.Icon({file: image})]];
  },
  navigationBarItems: { left: 'sidePanel' }
});

AppsModel = new EUI.ArrayModel([]);

AppsController = EUI.FrameController({
  hasNavigationBar: false,
  hasTabBar: false,
  model: AppsModel
});

AppsListController = EUI.ListController({
  hasNavigationBar: false,
  model: AppsModel,
  icon: 'apps',
  itemAtIndex: function(index) {
    var item = this.model.itemAtIndex(index);
    return {
      text: item.title,
      icon: item.icon
    };
  },
  selectedItemAtIndex: function(index) {
    this.split.right.promoteController(index);
    this.split.leftPanelVisible = false;
  }
});

SplitController = EUI.SplitController({
  model: new EUI.ArrayModel([
    new AppsListController(),
    new AppsController()
  ]),
});

EUI.app(new SplitController());

var tmr = setInterval(function() {
  AppsModel.pushItem(new Controller(icons[cnt], icons[cnt].toLowerCase()));
  if (++cnt == icons.length)
    clearInterval(tmr);
}, 2000);

