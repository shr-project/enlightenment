var EUI = require("eui");

Modelo = new EUI.ArrayModel([
  {text: 'Apple', badge: {text: '1', color: {red: 255}}},
  {text: 'Banana', badge: {text: '12', color: {red: 255, green: 255 }}},
  {text: 'Orange', badge: {text: '6', color: {red: 255, green: 128}}},
  {text: 'Watermelon', badge: {text: '1', color: {green: 255}}},
  {
    text: 'EFL',
    badge: {
     image: '/usr/local/share/elev8/data/images/logo_small.png',
     color: {red: 0, green: 0, blue: 0}
    }
  }
]);

var MyList = EUI.ListController({
  model: Modelo,
  itemAtIndex: function(index) {
    var item = this.model.itemAtIndex(index);
    return {
      text: item.text,
      badge: item.badge
    };
  },
});

EUI.app(new MyList());
