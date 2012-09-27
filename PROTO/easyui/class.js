/* Simple JavaScript Inheritance
 * By John Resig http://ejohn.org/
 * MIT Licensed.
 */
// Inspired by base2 and Prototype
var initializing = false, fnTest = /xyz/.test(function(){xyz;}) ? /\b_super\b/ : /.*/;
// The base Class implementation (does nothing)
Class = function(){};

Property = function(getter, setter, default_value) {
  this.getter = getter || function() { };
  this.setter = setter || function(v) { };
  this.default_value = default_value;
  return this;
};

// Create a new Class that inherits from this class
Class.extend = function(prop) {
  var _super = this.prototype;

  // Instantiate a base class (but only create the instance,
  // don't run the init constructor)
  initializing = true;
  var prototype = new this();
  initializing = false;

  // Copy the properties over onto the new prototype
  for (var name in prop) {
    // Check if we're overwriting an existing function
    if (typeof prop[name] == "function" &&
        typeof _super[name] == "function" &&
        fnTest.test(prop[name])) {
      prototype[name] = (function(name, fn){
        return function() {
          var tmp = this._super;

          // Add a new ._super() method that is the same method
          // but on the super-class
          this._super = _super[name];

          // The method only need to be bound temporarily, so we
          // remove it when we're done executing
          var ret = fn.apply(this, arguments);
          this._super = tmp;

          return ret;
        };
      })(name, prop[name]);
    } else if (prop[name] instanceof Property) {
      prototype['_' + name] = prop[name].default_value;
      prototype.__defineSetter__(name, prop[name].setter);
      prototype.__defineGetter__(name, prop[name].getter);
    } else {
      prototype[name] = prop[name];
    }
  }

  // The dummy class constructor
  function Class() {
    // All construction is actually done in the init method
    if (initializing) return;

    if (this.willInitialize)
      this.willInitialize.apply(this, arguments);
    if (this.init)
      this.init.apply(this, arguments);
    if (this.didInitialize)
      this.didInitialize.apply(this, arguments);
  }

  // Populate our constructed prototype object
  Class.prototype = prototype;

  // Enforce the constructor to be what we expect
  Class.prototype.constructor = Class;

  // And make this class extendable
  Class.extend = arguments.callee;

  return Class;
};

exports.Class = Class;
exports.Property = Property;
