var eui = require('eui');

NoteController = eui.FormController({
  init: function(model, index) {
    var item = model.itemAtIndex(index);
    this.title = item ? item.title : 'Note';

    this.model = model;
    this.index = index;
    this.editable = true;
  },
  fields: [
     [eui.widgets.Entry({field: 'title', scrollable: true, single_line: true})],
     [eui.widgets.Entry({field: 'note', scrollable: true})],
  ],
});

NotesController = eui.ListController({
  title: 'Notes',
  model: new eui.DBModel('note'),
  editable: true,
  selectedNavigationBarItem: function(item) {
    this.pushController(new NoteController(this.model));
  },
  selectedItemAtIndex: function(index) {
    this.pushController(new NoteController(this.model, index));
  },
  itemAtIndex: function(index) {
    return {
      text: this.model.itemAtIndex(index).title
    };
  },
});

eui.app(new NotesController());
