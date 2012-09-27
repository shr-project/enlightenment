xhr = require('http');
sax = require('sax-js');

var kDone = 4;

var default_options = {
  serialize_requests: true,
  number_of_queues: 10
}

var ajax_options = utils.clone(default_options, true);

var module_hooks = (function() {
  var all_modules = modules();
  var hooks = [];

  for (var module in all_modules) {
    if (!all_modules.hasOwnProperty(module)) continue;
    if (!all_modules[module].hasOwnProperty('module_hooks')) continue;
    var hook = all_modules[module].module_hooks.ajax;

    if(hook)
      hooks.push(hook);
  }

  var call = function(object, name, params) {
    for (var i = 0; i < hooks.length; ++i) {
      if (!hooks[i].hasOwnProperty(name)) continue;
      hooks[i][name].apply(object, params);
    }
  };

  return {
    call: call
  };
})();

timeout_callback = function(request) {
  if (request.readyState == kDone)
    return;

  request.abort();
  if (typeof(request.onreadystatechange) === 'function')
    request.onreadystatechange('timeout');
}

internal_queue = function() {
  var requests = [];
  var timer = undefined;
  var pending = 0;

  var thaw = function() { --pending; };
  var freeze = function() { ++pending; };

  var consume = function() {
    if (pending)
      return;
    var request = requests.pop();
    if (!request) {
      timer = clearInterval(timer);
      return;
    }
    freeze();

    var xhr = request[0];
    var params = request[1];

    if (xhr.timeout)
      setTimeout(timeout_callback.bind(this, xhr), xhr.timeout);

    xhr.send(params);
  };

  var queue = function(request, data) {
    requests.unshift([request, data]);
    if (timer === undefined)
      timer = setInterval(consume, 100);
  };

  var getPendingRequests = function() {
    return requests.length;
  };

  return {
    queue: queue,
    thaw: thaw,
    freeze: freeze,
    getPendingRequests: getPendingRequests
  };
};

queues = (function() {
  var queues = [];
  var queue = function(request, data){
    //queues in the first empty queue
    for (var i in queues) {
      if (!queues[i].getPendingRequests()) {
        queues[i].queue(request, data);
        return i;
      }
    }

    //if it's possible to create more queues, create and queue
    if (queues.length < ajax_options.number_of_queues){
      var len = queues.push(internal_queue());
      queues[len -1].queue(request, data);
      return len - 1;
    }

    //queues on the smaller queue
    var index;
    var pending = Number.MAX_VALUE;
    for (var i in queues) {
      if (queues[i].getPendingRequests() < pending) {
        pending = queues[i].getPendingRequests();
        index = i;
      }
    }
    queues[index].queue(request, data);
    return index;
  }

  var thaw = function(queue_index) {
    queues[queue_index].thaw();
  }

  return {
    queue: queue,
    thaw: thaw
  }
})();

xmltojson = function(request) {
  var objectIsEmpty = function(obj) {
    for (var i in obj)
      if (obj.hasOwnProperty(i))
        return false;
    return true;
  };

  var parser = sax.parser(true, {trim: true, normalize: true, lowercase: true});
  var xml = {};
  var stack = [];
  var queue = [];

  parser.onopentag = function(tag) {
    var node = xml;

    for (var i = 0; i < queue.length; ++i) {
      /* Find correct tag node position in JSON representation of XML */
      /* tree. Search is realized from root node to leaves.           */
      var inner_tag = queue[i];

      if (node[inner_tag.name].length)
        /* Inner tag has multiples items. Point to the last list node */
        node = node[inner_tag.name][node[inner_tag.name].length - 1];
      else
        /* Inner tag is a single item. Continue searching on tree. */
        node = node[inner_tag.name];
    }

    if (node[tag.name]) {
      /* Curret tag already exists on JSON representation. Change tag */
      /* representation from single item to item list and push tag.   */
      node[tag.name] = [].concat(node[tag.name]);
      node[tag.name].push({})
    } else {
      /* Tag is not on JSON representation. Allocate space to it. */
      node[tag.name] = {};
    }

    if (!objectIsEmpty(tag.attributes)) {
      if (node[tag.name].hasOwnProperty('length'))
        node[tag.name][node[tag.name].length - 1]['attributes'] = tag.attributes;
      else
        node[tag.name]['attributes'] = tag.attributes;
    }

    stack.push(tag);
    queue.push(tag);
  }.bind(this);

  parser.onclosetag = function(tagName) {
    stack.pop();
    queue.pop();
  }.bind(this);

  parser.ontext = function(text) {
    var node = xml;
    var stackTop = stack[stack.length - 1];

    for (var i = 0; i < queue.length; ++i) {
      /* Find correct tag node position in JSON representation of XML */
      /* tree. Search is realized from root node to leaves.           */
      var tag = queue[i];

      if (tag === stackTop)
        /* Tag is a leaf in XML tree. Stop searching. */
        break
      else if (node[tag.name].length)
        /* Tag has multiple items. Point to last item list. */
        node = node[tag.name][node[tag.name].length - 1];
      else
        /* Continue searching on tree. */
        node = node[tag.name];
    }

    if (!objectIsEmpty(node[stackTop.name]))
      node[stackTop.name]['#text'] = text;
    else
      node[stackTop.name] = text;
  }.bind(this);

  parser.onend = function() {
    request.responseXML = xml;
  }.bind(this);

  parser.write(request.responseText);
  parser.end();
}

ajax = function(url, options) {
  if (url === undefined)
    return null;

  var request = new xhr.XMLHttpRequest();
  var method = options.type || 'GET';
  var data = options.data || undefined;

  request.open(method, url);

  if (typeof(options.onSuccess) === 'function') {
    request.onreadystatechange = function(options, status) {
      var contentType = request.getResponseHeader('Content-Type');

      if (contentType === 'text/xml' || options.contentType === 'text/xml')
        xmltojson(this);

      options.onSuccess(this, status || 'success');
      module_hooks.call(this, 'onComplete', [request, options]);

      if (typeof(options.onComplete) === 'function')
        options.onComplete(this);

      if (this.queue_index !== undefined)
        queues.thaw(this.queue_index);
    }.bind(request, options);
  }

  module_hooks.call(this, 'beforeSend', [request, options]);

  if (typeof(options.beforeSend) === 'function')
    options.beforeSend(request, options);

  if (typeof(options.headers) === 'object') {
    for (var header in options.headers) {
      if (!options.headers.hasOwnProperty(header))
        continue;

      request.setRequestHeader(header, options.headers[header]);
    }
  }

  if (typeof(options.timeout) === 'number')
    request.timeout = options.timeout;

  var serialize_request;
  if (options.serialize_request !== undefined)
    serialize_request = options.serialize_request
  else
    serialize_request = ajax_options.serialize_requests;

  if (serialize_request) {
    request.queue_index = queues.queue(request, data);
  } else {
    if (request.timeout)
      setTimeout(timeout_callback.bind(this, request), request.timeout);
    request.send(data);
  }


  return request;
};

get = function(url, data, success) {
  return ajax(url, {
    data: data,
    onSuccess: success,
    type: 'GET'
  });
};

post = function(url, data, success) {
  return ajax(url, {
    data: data,
    onSuccess: success,
    type: 'POST'
  });
};

setup = function(options) {
  if (options) {
    for (var i in options)
      ajax_options[i] = options[i];
  } else {
    ajax_options = utils.clone(default_options, true);
  }
}

exports.get = get;
exports.post = post;
exports.ajax = ajax;
exports.setup = setup;
