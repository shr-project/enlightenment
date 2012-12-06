xhr = require('http');
sax = require('sax-js');

var kDone = 4;

var default_options = {
  serialize_requests: true,
  number_of_queues: 10,
  max_cache_size: 2 * 1024 * 1024, //2M
}

var ajax_options = utils.clone(default_options, true);

var cache = (function() {
  var cache = [];
  var eager_cache = [];
  var blacklist = [];
  var cache_size = 0;

  canCache = function(request, url) {
    if (cache[url])
      return false;

    //won't cache if there's no content-length or if it's too big
    var content_length = request.getResponseHeader('Content-Length');
    if (!content_length || (Number(content_length) > (ajax_options.max_cache_size * 0.5)))
      return false;

    //won't cache if cache-control says to not cache
    var cache_control = request.getResponseHeader('Cache-Control');
    if (cache_control && ((cache_control.indexOf('no-cache') != -1) ||
                           cache_control.indexOf('must-revalidate') != -1))
      return false;

    return true;
  }

  createMockRequest = function(request) {
    return {
      _responseHeaders: request.getAllResponseHeaders(),
      fromCache: true,
      status: request.status,
      responseText: request.responseText,
      readyState: kDone,
      getResponseHeader: function(status) {
        var pattern = new RegExp(status + ": .*");
        var result = pattern.exec(this._responseHeaders);
        if (result) {
          var header = result[0];
          return header.substring(header.indexOf(":") + 2); //2 due to space after colon
        }
      },
      getAllResponseHeaders: function() {
        return this._responseHeaders;
      },
      send: function() {
        if ((!this.aborted) && typeof(this.onreadystatechange) === 'function')
          setTimeout(this.onreadystatechange, 100);
      },
      open: function() {/*mock*/ }
    }
  }

  ejectLeastPointedEntries = function(size_to_free) {
    points = [];
    for (var i in cache)
      points.push({points: cache[i].points, url: i});

    points.sort(function(a, b) {return a.points - b.points});

    for (var i = 0; ((cache_size + size_to_free) > ajax_options.max_cache_size) && i < points.length; i++) {
      var entry_url = points[i].url;
      cache_size -= cache[entry_url].size;
      delete cache[points[i].url];
    }
  }

  cacheRequest = function(url, request) {
    if (!canCache(request, url)) {
      if (eager_cache[url])
        handleInvalidEagerCaching(url, request);

      return;
    }

    var content_length = Number(request.getResponseHeader('Content-Length'));

    if ((cache_size + content_length) > ajax_options.max_cache_size)
      ejectLeastPointedEntries(content_length);

    cache[url] = {points: 1, request: createMockRequest(request), size: content_length};
    cache_size = cache_size + content_length;

    if (eager_cache[url]) {
      notifyPendingRequests(url, cache[url].request);
      delete eager_cache[url];
    }
  }

  handleCacheMiss = function() {
    for (var i in cache)
      cache[i].points *= 0.9;
  }

  fetchCachedRequest = function(url) {
    if (cache[url]) {
      cache[url].points *= 1.1;
      return utils.clone(cache[url].request);
    }

    if (eager_cache[url]) {
      if (!eager_cache[url].pending)
        eager_cache[url].pending = [];

      var eager_mock_request = createEagerMockRequest();
      eager_cache[url].pending.push(eager_mock_request);
      return eager_mock_request;
    }

    handleCacheMiss();
  }

  eagerCacheRequest = function(url) {
    if (!blacklist[url])
      eager_cache[url] = {};
  }

  createEagerMockRequest = function(){
    return {
      fromCache: true,
      eagerlyCached: true,
      aborted: false,
      abort: function() {
        this.aborted = true;
      },
      setRequestHeader: function() {
        print('WARNING: You are trying to setRequestHeader for an eagerly cached request.\n' +
              'As you probably don\'t want it, disable cache for this request \n' +
              'defining, in the object options of this request, \'disableCache: true\'');
      },
      open: function() {/*mock*/},
      send: function() {/*mock*/},
      getResponseHeader: function() {/*mock*/},
      getAllResponseHeaders: function() {/*mock*/},
    }
  }

  updateEagerMockRequest = function(updated_mock, mock) {
    mock._responseHeaders = updated_mock._responseHeaders;
    mock.size = updated_mock.size;
    mock.status = updated_mock.status;
    mock.responseText = updated_mock.responseText;
    mock.readyState = updated_mock.readyState;
    mock.getResponseHeader = updated_mock.getResponseHeader;
    mock.getAllResponseHeaders = updated_mock.getAllResponseHeaders;
    mock.send = updated_mock.send;
    mock.invalidCache = updated_mock.invalidCache;
  }

  notifyPendingRequests = function(url, mock_request) {
    if (eager_cache[url].pending) {
      var pending = eager_cache[url].pending;

      for (var i in pending) {
        updateEagerMockRequest(mock_request, pending[i]);
        pending[i].send();
      }

      delete eager_cache[url].pending;
    }
  }

  handleInvalidEagerCaching = function(url, request) {
    blacklist[url] = true; //just to put something. What matters is the existence of blacklist[url]

    if (eager_cache[url].pending) {
      var mock_request = createMockRequest(request);
      mock_request.invalidCache = true;

      notifyPendingRequests(url, mock_request);
    }

    delete eager_cache[url];
  }

  return {
    fetchCachedRequest: fetchCachedRequest,
    eagerCacheRequest: eagerCacheRequest,
    cacheRequest: cacheRequest
  }
})();


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

send_request = function(request, data) {
  if (request.timeout)
    setTimeout(timeout_callback.bind(this, request), request.timeout);

  request.send(data);
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

    send_request(xhr, params);
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

parsejson = function(request) {
  try {
    request.responseJSON = JSON.parse(request.responseText);
  } catch (e) {
    request.responseJSON = undefined;
  }
};

isContentType = function(request, options, value) {
  if (options.contentType)
   return (options.contentType.indexOf(value) == 0)

  var contentType = request.getResponseHeader('Content-Type');
  if (contentType && contentType.indexOf(value) == 0)
    return true;

  return false;
}

flatten = function(obj) {
  if (!obj)
    return '';

  var output = [];
  for (var key in obj)
    output.push(key + '=' + obj[key]);

  return '?' + output.join('&');
}

ajax = function(url, options) {
  if (url === undefined)
    return null;

  var method = options.type || 'GET';
  var data = options.data || undefined;
  var request;

  if (method == 'GET')
    request = cache.fetchCachedRequest(url + flatten(data));

  if (!request) {
    request = new xhr.XMLHttpRequest();

    if (method == 'GET' && !options.disableCache)
      cache.eagerCacheRequest(url + flatten(data));
  }

  request.open(method, url);

  if (typeof(options.onSuccess) === 'function') {
    request.onreadystatechange = function(options, status) {
      if (isContentType(request, options, 'text/html'))
        xmltojson(this);
      else if (isContentType(request, options, 'application/json'))
        parsejson(this);

      options.onSuccess(this, status || 'success');
      module_hooks.call(this, 'onComplete', [request, options]);

      if (typeof(options.onComplete) === 'function')
        options.onComplete(this);

      if (this.queue_index !== undefined)
        queues.thaw(this.queue_index);

      if (method == 'GET' && !(request.fromCache || options.disableCache))
        cache.cacheRequest(url + flatten(data), request);
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

  //a cached request doesn't need to queue
  if (request.fromCache) {
    send_request(request);
    return request;
  }

  var serialize_request;
  if (options.serialize_request !== undefined)
    serialize_request = options.serialize_request
  else
    serialize_request = ajax_options.serialize_requests;

  if (serialize_request) {
    request.queue_index = queues.queue(request, data);
  } else {
    send_request(request, data);
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
