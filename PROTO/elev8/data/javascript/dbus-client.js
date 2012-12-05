var dbus = require('dbus');

var conn = new dbus.Connection();
var obj = conn.getObject('org.Enlightenment', '/org/enlightenment');
var proxy = obj.getProxy('org.enlightenment.Test');

proxy.addSignalHandler('Alive', function() { print('Alive', arguments); });

proxy.addSignalHandler('Hello', function() { print('Hello', arguments); });

proxy.SendBool(true)
    .onComplete(function(bool) { print('SendBool', bool == true ? 'Ok' : 'Fail'); });

proxy.send('SendBool', dbus.Boolean(false))
    .onComplete(function(bool) { print('SendBool', bool == false ? 'Ok' : 'Fail'); });

proxy.send('SendByte', dbus.Byte(123))
    .onComplete(function(_byte) { print('SendByte', _byte == 123 ? 'Ok' : 'Fail'); });

proxy.send('SendUint32', dbus.UInt32(12345))
    .onComplete(function(uint) { print('SendUint32', uint == 12345 ? 'Ok' : 'Fail'); });

proxy.send('SendInt32', dbus.Int32(54321))
    .onComplete(function(i) { print('SendInt32', i == 54321 ? 'Ok' : 'Fail'); });

proxy.send('SendInt16', dbus.Int16(12345))
    .onComplete(function(i) { print('SendInt16', i == 12345 ? 'Ok' : 'Fail'); });

proxy.send('SendDouble', dbus.Double(123.456))
    .onComplete(function(d) { print('SendDouble', d == 123.456 ? 'Ok' : 'Fail'); });

proxy.SendDouble(100)
    .onComplete(function(d) { print('SendDouble', d == 100 ? 'Ok' : 'Fail'); });

proxy.send('SendString', dbus.String('Test'))
    .onComplete(function(s) { print('SendString', s == 'Test' ? 'Ok' : 'Fail'); });

proxy.SendString('Test 2')
    .onComplete(function(s) { print('SendString', s == 'Test 2' ? 'Ok' : 'Fail'); });

proxy.send('AsyncTest')
    .onComplete(function(s) { print('AsyncTest:', s); });


var pending = proxy.SendBool(true);

pending.onComplete(function() { print('It should be canceled'); });
pending.onError(function(name, msg) {
    if (name == 'org.enlightenment.DBus.Canceled')
        print(msg);
    else
        print('Error:', name, ':', msg);
});
pending.cancel();


