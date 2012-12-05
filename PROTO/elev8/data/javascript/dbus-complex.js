
var dbus = require('dbus');

var conn = new dbus.Connection();
var obj = conn.getObject('com.profusion', '/com/profusion/Test');
var proxy = obj.getProxy('com.profusion.Test');
var properties = proxy.getProperties();

setInterval(function() {
    print("\nProperties", properties, "\n");
}, 5000);

properties.Resp2 = 'lalalas';

var introspect = obj.getProxy('org.freedesktop.DBus.Introspectable');

introspect.send('Introspect')
    .onComplete(function() { print('Introspect', arguments); });

proxy.send('ReceiveArray', ['aaaa', 'bbbb', 'cccc'])
    .onComplete(function() { print('ReceiveArray', arguments); });

proxy.send('SendArray')
    .onComplete(function() { print('SendArray', arguments); });

proxy.send('SendArrayInt')
    .onComplete(function() { print('SendArrayInt', arguments); });

proxy.send('PlusOne', dbus.Int32(1))
    .onComplete(function() { print('PlusOne', arguments); });

proxy.send('ReceiveArrayOfStringIntWithSize', dbus.Int32(5), [
    dbus.Struct('struct 10', dbus.Int32(11)),
    dbus.Struct('struct 20', dbus.Int32(22)),
    dbus.Struct('struct 30', dbus.Int32(33)),
    dbus.Struct('struct 40', dbus.Int32(44)),
    dbus.Struct('struct 50', dbus.Int32(55)),
]).onComplete(function () { print('ReceiveArrayOfStringIntWithSize', arguments); });

proxy.send('DoubleContainner', [
    dbus.Struct(dbus.Int32(11), dbus.Int32(12)),
    dbus.Struct(dbus.Int32(13), dbus.Int32(14)),
    dbus.Struct(dbus.Int32(15), dbus.Int32(16)),
    dbus.Struct(dbus.Int32(17), dbus.Int32(18)),
    dbus.Struct(dbus.Int32(19), dbus.Int32(10)),
], [
    dbus.Struct(dbus.Int32(21), dbus.Int32(22)),
    dbus.Struct(dbus.Int32(23), dbus.Int32(24)),
    dbus.Struct(dbus.Int32(25), dbus.Int32(26)),
    dbus.Struct(dbus.Int32(27), dbus.Int32(28)),
    dbus.Struct(dbus.Int32(29), dbus.Int32(20)),
]).onComplete(function () { print('DoubleContainner', arguments); });

