EUI = require('eui');
Class = require('class.js').Class;
OAuth = require('oauth.js').OAuth;
ajax = require('ajax.js');

var XMLHttpRequest = require('http').XMLHttpRequest;
var gToken = null;
var gTokenSecret = null;
var gCallbackUrl = "http://www.enlightenment.org/"

localStorage.database = 'twitter.eet';

var parseParametersResponse = function(response){
  result = {};
  var parameters = response.split('&');
  for (i in parameters){
    var equal_separated = parameters[i].split('=');
    result[equal_separated[0]] = equal_separated[1]
  }
  return result;
}

var arrayBetweenBoundaries = function(first, last) {
  var array = [];
  for (var value = first, i = 0; value <= last; value++, i++)
    array[i] = value;

  return array;
}

twitterAjax = {
  _baseTwitterAPIUrl: 'https://api.twitter.com/',
  _getTwitterAPIUrl: function(endpoint){
    var url = '';
     //if endpont starts with 'http', it must be a full url
    if (endpoint.substring(0, "http".length) == 'http')
      url += endpoint;
    else
      url += this._baseTwitterAPIUrl + endpoint;

    return url;
  },
  _oauthAccessor: function(token, tokenSecret) {
    var accessor = {consumerKey: 'HRlpe96oNRNksZOuW64KcA',
                  consumerSecret: 'G46XTOYqVXpcKbJ8dMEXDs9pQ8B1bM6Trkyjqe3I'}

    if (token) {
      accessor.token = token;
      accessor.tokenSecret = tokenSecret;
    } else if (gToken) {
      accessor.token = gToken;
      accessor.tokenSecret = gTokenSecret;
    }
    return accessor;
  },
  _oauthHeader: function(method, endpoint, parameters, token, tokenSecret) {
    var accessor = this._oauthAccessor(token, tokenSecret);
    var message = {action: this._getTwitterAPIUrl(endpoint), method: method}

    OAuth.setParameters(message, parameters);
    OAuth.completeRequest(message, accessor);

    return OAuth.getAuthorizationHeader('', message.parameters);
  },
  _request: function(endpoint, parameters, success, method, token, tokenSecret) {
    return ajax.ajax(this._getTwitterAPIUrl(endpoint), {
      //the 'oauth_callback' parameter must NOT be sent in the http body, but in
      //the Authorization field in the header
      data: parameters && parameters.oauth_callback ? null : parameters,
      blockUI: true,
      onSuccess: success,
      type: method,
      beforeSend: function (request, options) {
        var oauth_header = this._oauthHeader(method, endpoint, parameters,
                                            token, tokenSecret);
        request.setRequestHeader('Authorization', oauth_header);
      }.bind(this)
    });
  },
  get: function(endpoint, parameters, success) {
    return this._request(endpoint, parameters, success, 'GET');
  },
  post: function(endpoint, parameters, success, token, tokenSecret) {
    return this._request(endpoint, parameters, success, 'POST', token, tokenSecret);
  }
}

BaseTwitterModel = EUI.Model({
  init: function(){
    this.items = [];
  },
  itemAtIndex: function(index) {
    return this.items[index];
  },
  length: function() {
    return this.items.length;
  },
  _request: function(method, endpoint, params, callback) {
    if (endpoint.substring(0, "http".length) != 'http')
      endpoint = "1/" + endpoint + ".json";
    twitterAjax[method](endpoint, params, function(req) {
      if (req.status != '200') return;
      callback(JSON.parse(req.responseText));
    });
  },
  post: function(endpoint, params, callback) {
    this._request('post', endpoint, params, callback.bind(this));
  },
  get: function(endpoint, params, callback) {
    this._request('get', endpoint, params, callback.bind(this));
  },
  fetchImage: function(url, index) {
    var item = this.items[index];
    if (!item || item.icon || item.alreadyFetchingImage)
      return;

    item.alreadyFetchingImage = true;
    ajax.get(url, null, function(req) {
      var item = this.items[index];
      if (!item) return;

      item.icon = req.responseText;
      this.notifyListeners(index);
    }.bind(this));
  }
});

TimelineModel = BaseTwitterModel.extend({
  init: function(user_id){
    this._super();
    this.user_id = user_id;

    if (user_id)
      this.url = "statuses/user_timeline";
    else
      this.url = "statuses/home_timeline";
  },
  buildUrlParameters: function(parameters) {
    //when we have user_id, we always need to add 'user_id' and 'include_rts'
    //to some twitter requests that get data. this method simplifies this task

    var params = parameters || {};
    if (this.user_id) {
      params.user_id = this.user_id;
      params.include_rts = true;
    }

    params.include_entities = true;
    return params;
  },
  refresh: function(){
    this.get(this.url, this.buildUrlParameters(), function(tweets) {
      this.items = tweets;
      this.notifyListeners();
    });
  },
  itemAtIndex: function(index){
    var item = this.items[index];

    var url_image;
    if (item.retweeted_status)
      url_image = item.retweeted_status.user.profile_image_url;
    else
      url_image = item.user.profile_image_url;

    this.fetchImage(url_image, index);

    return item;
  },
  retweet: function(tweet){
    this.post('statuses/retweet/' + tweet.id_str, null, function(retweet) {
      //search and update tweet
      for (var i in this.items){
        if (this.items[i].id_str == retweet.retweeted_status.id_str){
          this.items[i] = retweet;
          this.notifyListeners(i);
          break;
        }
      }
    });
  },
  tweet: function(text, replying_tweet){
    var params = {status: text};
    if (replying_tweet)
      params.in_reply_to_status_id = replying_tweet.id_str;

    this.post('statuses/update', params, function(item) {
      this.items.unshift(item);
      this.notifyListeners(0); //just the first item needs to be updated
    });
  },
  appendTweets: function(tweets){
      var old_length = this.items.length;
      this.items = this.items.concat(tweets);
      this.notifyListeners(
        arrayBetweenBoundaries(old_length, this.items.length -1));
  },
  insertTweetsOnTop: function(tweets){
    this.items = tweets.concat(this.items);
    this.notifyListeners();
  },
  fetchOlderTweets: function(){
    //to get twitters older than the last one that we have
    var params = this.buildUrlParameters({max_id: this.items[this.items.length - 1].id_str});

    this.get(this.url, params, function(results) {
      this.notifyListeners(this.length + 1, 'finishSlowLoad');
      //exclude from results the first item, which is repetead
      this.appendTweets(results.slice(1));
   });

    this.notifyListeners(this.length + 1, 'beginSlowLoad');
  },
  fetchNewerTweets: function(){
    //to get twitters newer than the first one that we have
    var params = this.buildUrlParameters({since_id: this.items[0].id_str});

    this.get(this.url, params, function(new_tweets) {
      this.notifyListeners(-1, 'finishSlowLoad');
      this.insertTweetsOnTop(new_tweets);
    });
    this.notifyListeners(-1, 'beginSlowLoad');
  },
  untweet: function(index) {
    var id_tweet = this.items[index].id_str;
    this.post('1/statuses/destroy/' + id_tweet, null, function() {
      this.items.splice(index, 1);
      this.notifyListeners();
    });
  }
});

TweetController = EUI.TableController({
  editable: true,
  init: function(model, replying_tweet){
    this.title = 'What\'s happening?';
    this.replying_tweet = replying_tweet;
    var text = ''
    if (this.replying_tweet)
      text = '@' + this.replying_tweet.user.screen_name + ' ';

    this.model = model;
    this.fields = [
      [EUI.widgets.Entry({scrollabe: true, field: 'text', text: text})]
    ]
  },
  navigationBarItems: { right: 'Tweet' },
  selectedNavigationBarItem: function(item) {
    if (item !== 'Tweet')
      return;

    var tweet = this.getValues().text;
    if (tweet.length > 0)
      this.model.tweet(tweet, this.replying_tweet);
    this.popController();
  },
  didChangeEntry: function(){
    var chars_left = 140 - this.getValues().text.length;
    if (chars_left == 140)
      this.title = "What's happening?";
    else
      this.title = chars_left;
  }
});

SearchTimelineModel = TimelineModel.extend({
  init: function(text) {
    this._super();

    this.url = 'http://search.twitter.com/search.json';
    this.query = text;
  },
  refresh: function(){
    this.get(this.url, {q: this.query, include_entities: true}, function(result) {
      this.items = result.results;
      this.notifyListeners();
    });
  },
  itemAtIndex: function(index) {
    var item = this.items[index];
    this.fetchImage(item.profile_image_url, index);

    if (!item.user)
      item.user = {
                    name: item.from_user_name,
                    screen_name: item.from_user_name,
                    id_str: item.from_user_id_str
                  };
    return item;
  },
  fetchOlderTweets: function() {
    var params = {q: this.query, include_entities: true,
      max_id: this.items[this.items.length -1].id_str}

    this.get(this.url, params, function(result) {
      this.notifyListeners(this.length + 1, 'finishSlowLoad');

      //exclude from results the first item, which is repetead
      this.appendTweets(result.results.slice(1));
    });
    this.notifyListeners(this.length + 1, 'beginSlowLoad');
  },
  fetchNewerTweets: function() {
    var params = {q: this.query, include_entities: true,
      since_id: this.items[0].id_str}

    this.get(this.url, params, function(result) {
      this.notifyListeners(-1, 'finishSlowLoad');

      this.insertTweetsOnTop(result.results);
    });
    this.notifyListeners(-1, 'beginSlowLoad');
  }
});

SearchController = EUI.TableController({
  editable: true,
  init: function() {
    this.fields = [
      [EUI.widgets.Entry({scrollable: true, field: 'search', single_line: true})]
    ];
  },
  navigationBarItems: { right: 'Search' },
  selectedNavigationBarItem: function(item) {
    if (item !== 'Search')
      return;

    var text = this.getValues().search;
    this.pushController(new TimelineController(new SearchTimelineModel(text), 'search', text));
  },
  saveSearchQuery: function(text) {
    this.parent.saveSearchQuery(text);
    this.popController();
  },
  title: 'Search',
  icon: 'search'
});

SavedSearchController = EUI.ListController({
  title: 'Searches',
  model: new EUI.DBModel('savedsearch'),
  itemAtIndex: function(index) {
    return {
      text: this.model.itemAtIndex(index).text
    };
  },
  selectedItemAtIndex: function(index) {
    var text = this.model.itemAtIndex(index).text;
    this.pushController(new TimelineController(new SearchTimelineModel(text), 'tweets', text));
  },
  navigationBarItems: {right: 'New'},
  selectedNavigationBarItem: function(item) {
    if (item == 'New')
      this.pushController(new SearchController());
  },
  contextMenuItems: ['Remove'],
  selectedContextMenuItem: function(menuItem, index) {
    if(menuItem == 'Remove')
      this.model.deleteItemAtIndex(index);
  },
  saveSearchQuery: function(text) {
    this.model.insert({ text: text });
  }
});

AuthController = EUI.TableController({
  init: function(token, tokenSecret){
    this.title = 'Sign In';
    this.token = token;
    this.tokenSecret = tokenSecret;
    this.fields = [
                    [EUI.widgets.Web({uri:
                      'https://api.twitter.com/oauth/authenticate?oauth_token='
                      + token
                    })]
                  ]
  },
  didChangeUri: function(uri) {
    //if it goes to the callback page, it means that it was authenticated
    if (uri.uri.substring(0, gCallbackUrl.length) == gCallbackUrl){
      var parsedParameters = parseParametersResponse(
        uri.uri.substring(uri.uri.indexOf('?') + 1));

      if (parsedParameters.oauth_token == this.token) {
        this.obtainAccessToken(parsedParameters.oauth_verifier);
      } else {
        //TODO something went wrong, must restart the oauth process
      }
    }
  },
  obtainAccessToken: function(oauth_verifier) {
    twitterAjax.post('oauth/access_token', {oauth_verifier: oauth_verifier}, function(req) {
      if (req.status != '200') return;

      var parsedResponse = parseParametersResponse(req.responseText);
      gToken = parsedResponse.oauth_token;
      gTokenSecret = parsedResponse.oauth_token_secret;
      gScreenName = parsedResponse.screen_name;
      gUserID = parsedResponse.user_id;

      localStorage.setItem('gToken', gToken);
      localStorage.setItem('gTokenSecret', gTokenSecret);
      localStorage.setItem('gScreenName', gScreenName);
      localStorage.setItem('gUserID', gUserID);

      this.popController();
    }.bind(this), this.token, this.tokenSecret);
  }
});

BrowserController = EUI.WebController({});

TimelineItemBuilder = (function() {
  var formatTweet = function(tweet) {
    var user = '';

    if (tweet.retweeted_status) {
      user += tweet.retweeted_status.user.name;

      if (tweet.user.id_str != gUserID)
        user += ' [Retweeted by ' + tweet.user.name + ']';
    } else
      user += tweet.user.name;

    if (tweet.retweeted)
      user += ' (Retweeted)'

    return {
      text: tweet.text,
      text_sub: "By " + user,
      icon: tweet.icon
    }
  }

  return {
    formatTweet: formatTweet
  }
})();

ProfileDataModel = BaseTwitterModel.extend({
  init: function(user_id) {
    this._super();
    this.user_id = user_id;
  },
  getUserTweets: function() {
    this.get("statuses/user_timeline",
                    {user_id: this.user_id, count: 5}, function(tweets) {
      this.items['tweets'] = tweets;
      this.notifyListeners('tweets');
    });
  },
  getUserProfile: function() {
    this.get('users/show', {user_id: this.user_id}, function(profile) {
      this.items['profile'] = profile;

      this.notifyListeners('profile');
    });
  }
});

// This is not a true model. It just provides a nice way to controller
// show the data
BaseProfileFakeModel = EUI.Model({
  init: function(user_id) {
    this.user_id = user_id;
    this.items = [];

    this.model = new ProfileDataModel(user_id);
    this.model.addListener(this);

    this.model.getUserProfile();
    this.model.getUserTweets();

    this.readyToShow = false;
  },
  didChangeModel: function(index) {
    //'index' can be 'profile' or 'tweets', so, this.profile or this.tweets will be valid
    this[index] = this.model.itemAtIndex(index);
    if (this.profile)
      this.title = this.profile.name;
    if (this.readyToShow)
      this.updateItems();
    else
      this.readyToShow = true;
  },
  updateItems: function() {
    this.items = [
      {
        text: this.profile.name,
        text_sub: '@' + this.profile.screen_name
      },
      {
        style: 'full',
        content: EUI.widgets.Table({
          hint_min: {width: 100, height: 100},
          elements: {
            description: {row: 0, col: 0,
              element: this.createLabel(this.profile.description.replace(/\r/g, '<br/>'))},
            location: {row: 1, col: 0,
              element: this.createLabel(this.profile.location)},
            url: {row: 2, col: 0,
              element: this.createLabel(this.profile.url || '<i>Not available</i>')}
          }
        })
      },
      {
        style: 'full',
        content: EUI.widgets.Table({
          hint_min: {width: 50, height: 50},
          elements: {
            tweets: {row: 3, col: 0,
              element: this.createLabel('Tweets: <b>' + this.profile.statuses_count + '</b>')},
            following: {row: 5, col: 0,
              element: this.createLabel('Following: <b>' + this.profile.friends_count + '</b>')},
            followers: {row: 6, col: 0,
              element: this.createLabel('Followers: <b>' + this.profile.followers_count + '</b>')}
          }
        })
      },
    ];

    for (var i in this.tweets) {
      var tweet = TimelineItemBuilder.formatTweet(this.tweets[i]);
      tweet.group = 'Tweets';
      tweet.tag = 'tweet',
      this.items.push(tweet);
    }
    //Add more tweets button
    this.items.push({
      text: 'More tweets',
      tag: 'more-tweets',
      icon: 'arrow_right',
      group: 'Tweets'
    });

    this.notifyListeners();
  },
  createLabel: function(text) {
    return EUI.widgets.Label({
      label: text,
      single_line: true
    });
  },
  length: function() {
    return this.items.length;
  },
  itemAtIndex: function(index) {
    var item = this.items[index];
    if (index == 0 && !item.icon)
      this.getProfileImage();

    if (item.tag == 'tweet' && !item.file && this.items[0].icon)
      item.icon = this.items[0].icon;

    return item;
  },
  getProfileImage: function() {
    //gets profile image

    ajax.get(this.profile.profile_image_url, null, function(req) {
      if (req.status != '200') return;
      var item = this.items[0];

      item.icon = req.responseText;
      this.notifyListeners();
    }.bind(this));
  }
});

UserProfileFakeModel = BaseProfileFakeModel.extend({
  updateItems: function() {
    this._super();

    //[un]follow action
    this.items.push({
      text: this.profile.following ? 'Unfollow' : 'Follow',
      tag: 'follow-action',
      icon: 'arrow_right',
      group: 'Actions'
    });

    this.notifyListeners();
  },
  followUnfollow: function() {
    var url;
    if (this.profile.following)
      url = '1/friendships/destroy.json';
    else
      url = '1/friendships/create.json';

    twitterAjax.post(url, {user_id: this.profile.id_str},
                     this.callbackFollowUnfollow.bind(this));
  },
  callbackFollowUnfollow: function(req) {
    if (req.status != 200) return;

    //update profile
    this.profile = JSON.parse(req.responseText);
    this.items[this.items.length - 1].text =
      this.profile.following ? 'Unfollow' : 'Follow';
    this.notifyListeners(this.items.length - 1);
  },
});

LoggedInUserProfileFakeModel = BaseProfileFakeModel.extend({
  updateItems: function() {
    this._super();

    //signout action
    this.items.push({
      text: 'Sign out',
      tag: 'sign-out',
      icon: 'arrow_right',
      group: 'Actions'
    });
  },
  signOut: function() {
    gToken = gTokenSecret = null;

    localStorage.removeItem('gToken');
    localStorage.removeItem('gTokenSecret');
    localStorage.removeItem('gScreenName');
    localStorage.removeItem('gUserID');
  }
});

BaseProfileController = EUI.ListController({
  style: 'text_label',
  mode: 'compress',
  updateView: function(indexes, hint) {
    this._super(indexes, hint);

    if (this.model.title)
      this.title = this.model.title;
  },
  itemAtIndex: function(index) {
    return this.model.itemAtIndex(index);
  },
  selectedItemAtIndex: function(index) {
    if (this.itemAtIndex(index).tag == 'more-tweets')
        this.pushController(new TimelineController(
           new TimelineModel(this.model.profile.id_str),
           'tweets', this.model.profile.screen_name));
  }
});

UserProfileController = BaseProfileController.extend({
  init: function(user_id) {
    this.model = new UserProfileFakeModel(user_id);
  },
  selectedItemAtIndex: function(index) {
    if (this.model.itemAtIndex(index).tag == 'follow-action'){
      this.model.followUnfollow();
      return;
    }

    this._super(index);
  },
});

LoggedInUserProfileController = BaseProfileController.extend({
  init: function(){
    this.title = 'Me';

    this.model = new LoggedInUserProfileFakeModel(gUserID);
  },
  selectedItemAtIndex: function(index) {
    if (this.model.itemAtIndex(index).tag == 'sign-out') {
      this.model.signOut();

      //TODO what to do next? show the authentication page?
      return;
    }

    this._super(index);
  }
});

TimelineController = EUI.ListController({
  icon: 'go-home',
  style: 'text_label',
  mode: 'compress',
  init: function(model, type, title){
    this.model = model;
    this.type = type;
    this.title = title;
    this.contextMenuDirection = 'vertical';

    gToken = localStorage.getItem('gToken');
    gTokenSecret = localStorage.getItem('gTokenSecret');
    gScreenName = localStorage.getItem('gScreenName');
    gUserID = localStorage.getItem('gUserID');

    if (!gToken)
      twitterAjax.post('oauth/request_token', {oauth_callback: gCallbackUrl}, function(req) {
        if (req.status != '200') return;

        var parsedResponse = parseParametersResponse(req.responseText);
        var token = parsedResponse.oauth_token;
        var tokenSecret = parsedResponse.oauth_token_secret;

        this.pushController(new AuthController(token, tokenSecret));
      }.bind(this));
    else
      this.model.refresh();
  },
  itemAtIndex: function(index){
    var item = this.model.itemAtIndex(index);
    return TimelineItemBuilder.formatTweet(item);
  },
  navigationBarItems: function() {
    if (this.type === 'home')
      return { left: 'Refresh', right: 'Tweet' };
    if (this.type === 'search' )
      return { right: 'Save' };
  },
  selectedNavigationBarItem: function(item) {
    if (item === 'Tweet')
      this.pushController(new TweetController(this.model));
    else if (item === 'Refresh')
      this.model.refresh();
    else if (item === 'Save') {
      this.parent.saveSearchQuery(this.title);
      this.popController();
    }
  },
  contextMenuItems: function(itemIndex) {
    var menuItems = ['Reply'];
    var tweet = this.model.itemAtIndex(itemIndex);

    menuItems.push({label: "See @" + tweet.user.screen_name,
                  screen_name: tweet.user.screen_name, id_str: tweet.user.id_str});

    //shows urls, medias and mentioned users embedded in this tweet
    var entities = tweet.entities.user_mentions;
    for (var i in entities)
      menuItems.push({label: "See @" + entities[i].screen_name,
                      screen_name: entities[i].screen_name, id_str: entities[i].id_str});

    entities = tweet.entities.urls.concat(tweet.entities.media || []);
    for (var i in entities)
      menuItems.push({label: "Go to " + entities[i].url, url: entities[i].url});

    if (tweet.retweeted)
      menuItems.unshift('Unretweet');
    else if (tweet.user.id_str != gUserID)
      menuItems.unshift('Retweet');

    //second comparison needed because when the user retweets, the
    //twitter is returned with user's id, instead of the id of the one
    //who tweeted id
    if (tweet.user.id_str == gUserID && !tweet.retweeted)
      menuItems.unshift('Untweet');

    return menuItems;
  },
  selectedContextMenuItem: function(menuItem, listIndex) {
    var tweet = this.model.itemAtIndex(listIndex);
    switch(menuItem) {
      case 'Reply':
        this.pushController(new TweetController(this.model, tweet));
        break;
      case 'Retweet':
        this.model.retweet(tweet);
        break;
      case 'Untweet':
        this.model.untweet(listIndex);
        break;
      case 'Unretweet':
        //TODO ^
        break;
      default:
        //must be a url or a profile
        if (menuItem.url)
          this.pushController(new BrowserController(menuItem.url));
        else {
          if (menuItem.id_str == gUserID)
            this.pushController(new LoggedInUserProfileController());
          else
            this.pushController(new UserProfileController(menuItem.id_str));
        }
    }
  },
  didScrollOverEdge: function(edge) {
    //TODO maybe, with tweaks in usability, it will never show an empty timeline,
    //thus this check won't be necessary.
    if (!this.model.length) return;

    if (edge == 'top')
      this.model.fetchNewerTweets();
    else if (edge == 'bottom')
      this.model.fetchOlderTweets();
  }
});

Gralha = EUI.TabController({
  model: new EUI.ArrayModel([
    new TimelineController(new TimelineModel(), 'home', 'Home'),
    new SavedSearchController(),
    new LoggedInUserProfileController(gUserID)
  ]),
  title: 'Gralha',
  titleVisible: false,
  tabPosition: 'bottom'
});

EUI.app(new Gralha());
