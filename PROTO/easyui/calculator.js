EUI = require("eui");

var button = function(label, field) {
  var res = {label: label}
  if (field) res.field = field;
  return new EUI.widgets.Button(res);
}

var display = new EUI.widgets.Entry({text: '0', field: 'display'});

BrainModel = EUI.Model({
  init: function(entries) {
    this.memory = 0;
    this.previous = 0;
    this.value = '0';
    this.clear = 'AC';
    this.show = 'previous';
  },
  itemAtIndex: function(index) {
    var result = { clear: this.clear };
    switch (this.show) {
      case 'memory':
      case 'value': result.display = this.value; break;
      case 'previous': result.display = this.previous; break;
      default: result.display = 'Erro';
    }
    return result;
  },
  length: function() {
    return 1;
  },
  deleteItemAtIndex: function(index) {
    this.value = 0;
    this.notifyListeners();
  },
  updateItemAtIndex: function(index, data) {
    var next_operator;
    switch (data.key) {
      case '+/-':
        this.value = new String(-1 * Number(this.value));
        break;
      case 'C':
        this.value = '0';
        this.clear = 'AC';
        this.show = 'value';
        break;
      case 'AC':
        this.value = '0';
        this.previous = 0;
        this.show = 'previous';
        break;
      case 'MC': this.memory = 0; break;
      case 'MR':
        this.show = 'memory';
        this.value = this.memory;
        break;
      case 'M+':
        this.show = 'memory';
        this.memory += Number(this.value);
        break;
      case 'M-':
        this.show = 'memory';
        this.memory -= Number(this.value);
        break;
      case '+':
      case '-':
      case '*':
      case '/':
        if (this.show == 'value') {
          next_operator = data.key;
        } else {
          this.operator = data.key;
          this.value = this.previous;
          return;
        }
      case '=':
        switch (this.operator) {
          case '+': this.previous += Number(this.value); break;
          case '-': this.previous -= Number(this.value); break;
          case '*': this.previous *= Number(this.value); break;
          case '/':
            if (this.value == '0') {
              this.show = 'erro';
              this.notifyListeners();
              return;
            }
            this.previous /= Number(this.value);
            break;
          case '=':
          default: this.previous = Number(this.value); break;
        }

        if (next_operator)
          this.operator = next_operator;

        this.show = 'previous';
        break;

      case '.':
        if (this.value.indexOf(data.key) >= 0)
          break;

        default:

          if (this.show != 'value') {
            this.value = '0';
            this.show = 'value';
          }

          this.clear = 'C';
          this.value += data.key;

          if ((this.value.length > 1) && (this.value[0] == '0') && (this.value[1] != '.'))
            this.value = this.value.slice(1);
    }
    this.notifyListeners();
  },
});

CalculatorController = EUI.TableController({
  model: new BrainModel(),
  index: 0,
  fields: [
    [display,		   '>',		  '>',		'>'],
    [button("MC"),	   button("M+"),  button("M-"),	button("MR")],
    [button('C', 'clear'), button("+/-"), button('/'),	button('*')],
    [button('7'),	   button('8'),	  button('9'),	button('-')],
    [button('4'),	   button('5'),   button('6'),	button('+')],
    [button('1'),	   button('2'),	  button('3'),	button('=')],
    [button('0'),	   '>',		  button('.'),	'V']
  ],
  didClickOnElement: function(item) {
    this.model.updateItemAtIndex(this.index, {key: item.label});
  },
});

EUI.app(new CalculatorController());
