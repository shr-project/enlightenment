

d = require('dbus');
print('listing properties in module d');
for (prop in d) {
  print(prop);
}

print('creating connection');
c = new d.Connection();
print('connection at', c);

print('getting object from connection');
o = c.getObject("org.gnome.VolumeControlApplet", "/org/gnome/VolumeControlApplet");
print('object at', o);


print('getting proxy from object');
p = o.getProxy("org.freedesktop.DBus.Properties");
print('proxy at', p);

print('adding signal handler');
h = p.addSignalHandler('PropertiesChanged', function() {
  print('prop changed. yay');
});
print('signal handler added at', h);

