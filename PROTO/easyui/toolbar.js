var EUI = require('eui');

MyController = EUI.TableController({
  titleVisible: false,
  init: function() {
    this.fields = [[
      EUI.widgets.Button({
        icon: EUI.widgets.Icon({file: 'arrow_up'}),
        on_click: function() {
          this.viewContent.signal_emit("top,toolbar", "");
        }.bind(this)
      }), '>'
    ],[
      EUI.widgets.Button({
        icon: EUI.widgets.Icon({file: 'refresh'}),
        on_click: function() {
          this.viewContent.signal_emit("show,toolbar", "");
        }.bind(this)
      }),
      EUI.widgets.Button({
        icon: EUI.widgets.Icon({file: 'close'}),
        on_click: function() {
          this.viewContent.signal_emit('hide,toolbar', "");
        }.bind(this)
      })
    ],[
      EUI.widgets.Button({
        icon: EUI.widgets.Icon({file: 'arrow_down'}),
        on_click: function() {
          this.viewContent.signal_emit("bottom,toolbar", "");
        }.bind(this)
      }), '>'
    ]];
  },
  toolbarItems: [
    { label: 'top', icon: 'arrow_up' },
    { label: 'bottom', icon: 'arrow_down' },
    { label: 'hide', icon: 'close' }
  ],
  selectedToolbarItem: function(item) {
    this.viewContent.signal_emit(item.label + ',toolbar', '');
  }
});

EUI.app(new MyController());
