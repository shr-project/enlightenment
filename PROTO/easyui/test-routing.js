

eui = require('eui');


assert = function(value, expected) {
  if (value === expected) {
    print(value, " === ", expected, "; PASS");
    return;
  }
  
  print(value, " !== ", expected, "; FAIL");
}

assert(eui.Routing.edit("image/gif", "test1.txt"), true);
assert(eui.Routing.view("text/html", "multiple.html"), true);
assert(eui.Routing.edit("something/not-in-db", "test1.txt"), false);
assert(eui.Routing.share("anything/goes-here", "test3"), true);
