EUI  = require('eui');

var path     = environment['HOME'];
var patterns = ["*.jpg", "*.jpeg", "*.png"];

FileController = EUI.ListController({
  init: function(path, patterns) {
    this.title = path;
    this.model = new EUI.FileModel(path, patterns);
  },
  selectedItemAtIndex: function(index) {
    var item = this.model.itemAtIndex(index);

    if (!item.isFile)
      this.pushController(new FileController(item.path, patterns));
  },
  itemAtIndex: function(index) {
    var item = this.model.itemAtIndex(index);

    return {
      text: item.name,
      icon: (item.isFile) ? undefined : "folder"
    };
  }
});

EUI.app(new FileController(path, patterns));
