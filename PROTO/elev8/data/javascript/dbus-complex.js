
var dbus = require('dbus');

var conn = new dbus.Connection();
var obj = conn.getObject('com.profusion', '/com/profusion/Test');
var proxy = obj.getProxy('com.profusion.Test');
var properties = obj.getProxy('org.freedesktop.DBus.Properties');
var introspect = obj.getProxy('org.freedesktop.DBus.Introspectable');

introspect.call('Introspect', function() {
    print('Introspect', arguments);
});

properties.addSignalHandler('PropertiesChanged', function() {
    print('PropertiesChanged', arguments);
});

properties.call('Get', 'com.profusion.Test', 'int32', function() {
    print('GetAll', arguments);
});

properties.call('Get', 'com.profusion.Test', 'text', function() {
    print('GetAll', arguments);
});

properties.call('Get', 'com.profusion.Test', 'st', function() {
    print('GetAll', arguments);
});

properties.call('GetAll', 'com.profusion.Test', function() {
    print('GetAll', arguments);
});

properties.call('Set', 'com.profusion.Test', 'Resp2', dbus.Variant('lalala'), function() {
    print('Resp2', arguments);
});

proxy.call('ReceiveArray', ['aaaa', 'bbbb', 'cccc'] , function() {
    print('ReceiveArray', arguments);
});

proxy.call('SendArray', function() {
    print('SendArray', arguments);
});

proxy.call('SendArrayInt', function() {
    print('SendArrayInt', arguments);
});

proxy.call('PlusOne', dbus.Int32(1), function() {
    print('PlusOne', arguments);
});

proxy.call('ReceiveArrayOfStringIntWithSize', dbus.Int32(5), [
    dbus.Struct('struct 10', dbus.Int32(11)),
    dbus.Struct('struct 20', dbus.Int32(22)),
    dbus.Struct('struct 30', dbus.Int32(33)),
    dbus.Struct('struct 40', dbus.Int32(44)),
    dbus.Struct('struct 50', dbus.Int32(55)),
], function () { print('ReceiveArrayOfStringIntWithSize', arguments); });

proxy.call('DoubleContainner', [
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
], function () { print('DoubleContainner', arguments); });

