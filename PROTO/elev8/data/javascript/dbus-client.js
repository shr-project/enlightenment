var dbus = require('dbus');

var conn = new dbus.Connection();
var obj = conn.getObject('org.Enlightenment', '/org/enlightenment');
var proxy = obj.getProxy('org.enlightenment.Test');

proxy.addSignalHandler('Alive', function() {
    print('Alive', arguments);
});

proxy.addSignalHandler('Hello', function() {
    print('Hello', arguments);
});

proxy.call('SendBool', true, function(bool) {
    print('SendBool', bool == true ? 'Ok' : 'Fail');
});

proxy.call('SendBool', dbus.Boolean(false), function(bool) {
    print('SendBool', bool == false ? 'Ok' : 'Fail');
});

proxy.call('SendByte', dbus.Byte(123), function(_byte) {
    print('SendByte', _byte == 123 ? 'Ok' : 'Fail');
});

proxy.call('SendUint32', dbus.UInt32(12345), function(uint) {
    print('SendUint32', uint == 12345 ? 'Ok' : 'Fail');
});

proxy.call('SendInt32', dbus.Int32(54321), function(i) {
    print('SendInt32', i == 54321 ? 'Ok' : 'Fail');
});

proxy.call('SendInt16', dbus.Int16(12345), function(i) {
    print('SendInt16', i == 12345 ? 'Ok' : 'Fail');
});

proxy.call('SendDouble', dbus.Double(123.456), function(d) {
    print('SendDouble', d == 123.456 ? 'Ok' : 'Fail');
});

proxy.call('SendDouble', 100, function(d) {
    print('SendDouble', d == 100 ? 'Ok' : 'Fail');
});

proxy.call('SendString', dbus.String('Test'), function(s) {
    print('SendString', s == 'Test' ? 'Ok' : 'Fail');
});

proxy.call('SendString', 'Test 2', function(s) {
    print('SendString', s == 'Test 2' ? 'Ok' : 'Fail');
});

proxy.call('AsyncTest', undefined, function(s) { print('AsyncTest:', s); });
