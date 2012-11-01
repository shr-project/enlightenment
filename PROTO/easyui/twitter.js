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

TimelineModel = EUI.Model({
  items: [],
  init: function(user_id){
    this.user_id = user_id;

    if (user_id)
      this.url = "1/statuses/user_timeline.json";
    else
      this.url = "1/statuses/home_timeline.json";
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
    twitterAjax.get(this.url, this.buildUrlParameters(), function(req) {
      if (req.status != '200') return;

      this.items = JSON.parse(req.responseText);
      this.notifyControllers();
    }.bind(this));
  },
  length: function(){
    return this.items.length;
  },
  itemAtIndex: function(index){
    var item = this.items[index];

    if (item && !item.file && !item.alreadyFetchingImage){
      item.alreadyFetchingImage = true;
      var url_image;
      if (item.retweeted_status)
        url_image = item.retweeted_status.user.profile_image_url;
      else
        url_image = item.user.profile_image_url;

      ajax.get(url_image, null, function(req) {
        var item = this.items[index];
        if (!item) return;

        item.file = req.responseText;
        this.notifyControllers(index);
      }.bind(this));
    }

    return item;
  },
  retweet: function(tweet){
    twitterAjax.post('1/statuses/retweet/' + tweet.id_str + '.json', null, function(req) {
      if (req.status != '200') return;

      retweet = JSON.parse(req.responseText);

      //search and update tweet
      for (var i in this.items){
        if (this.items[i].id_str == retweet.retweeted_status.id_str){
          this.items[i] = retweet;
          this.notifyControllers(i);
          break;
        }
      }
    }.bind(this));
  },
  tweet: function(text, replying_tweet){
    var params = {status: text};
    if (replying_tweet)
      params.in_reply_to_status_id = replying_tweet.id_str;

    twitterAjax.post('1/statuses/update.json', params, function(req) {
      if (req.status != '200') return;

      item = JSON.parse(req.responseText);
      this.items.unshift(item);
      this.notifyControllers(0); //just the first item needs to be updated
    }.bind(this));
  },
  fetchOlderTweets: function(){
    //to get twitters older than the last one that we have
    var params = this.buildUrlParameters({max_id: this.items[this.items.length - 1].id_str});

    twitterAjax.get(this.url, params, function(req) {
      this.notifyControllers(this.length + 1, 'finishSlowLoad');
      if (req.status != '200') return;

      //exclude from results the first item, which is repetead
      var results = JSON.parse(req.responseText).slice(1);

      var old_length = this.items.length;
      this.items = this.items.concat(results);
      this.notifyControllers(
        arrayBetweenBoundaries(old_length, this.items.length -1));
    }.bind(this));

    this.notifyControllers(this.length + 1, 'beginSlowLoad');
  },
  fetchNewerTweets: function(){
    //to get twitters newer than the first one that we have
    var params = this.buildUrlParameters({since_id: this.items[0].id_str});

    twitterAjax.get(this.url, params, function(req) {
      this.notifyControllers(-1, 'finishSlowLoad');
      if (req.status != '200') return;

      var new_tweets =  JSON.parse(req.responseText);
      this.items = new_tweets.concat(this.items);
      this.notifyControllers();
    }.bind(this));
    this.notifyControllers(-1, 'beginSlowLoad');
  },
  untweet: function(index) {
    var id_tweet = this.items[index].id_str;
    twitterAjax.post('1/statuses/destroy/' + id_tweet + '.json', null, function(req) {
      if (req.status != '200') return;

      this.items.splice(index, 1);
      this.notifyControllers();
    }.bind(this));
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
  }
});

SearchTimelineModel = TimelineModel.extend({
  init: function(text) {
    this.url = 'http://search.twitter.com/search.json';
    this.query = text;
  },
  refresh: function(){
    twitterAjax.get(this.url, {q: this.query, include_entities: true}, function(req) {
      if (req.status != '200') return;

      this.items = JSON.parse(req.responseText).results;
      this.notifyControllers();
    }.bind(this));
  },
  itemAtIndex: function(index) {
    var item = this.items[index];

    if (!item.file && !item.alreadyFetchingImage){
      item.alreadyFetchingImage = true;
      ajax.get(item.profile_image_url, null, function(req) {
        var item = this.items[index];
        if (!item) return;

        item.file = req.responseText;
        this.notifyControllers(index);
      }.bind(this));
    }

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

    twitterAjax.get(this.url, params, function(req) {
      this.notifyControllers(this.length + 1, 'finishSlowLoad');
      if (req.status != '200') return;

      //exclude from results the first item, which is repetead
      var results = JSON.parse(req.responseText).results.slice(1);

      var old_length = this.items.length;
      this.items = this.items.concat(results);
      this.notifyControllers(arrayBetweenBoundaries(old_length, this.items.length -1));
    }.bind(this));
    this.notifyControllers(this.length + 1, 'beginSlowLoad');
  },
  fetchNewerTweets: function() {
    var params = {q: this.query, include_entities: true,
      since_id: this.items[0].id_str}

    twitterAjax.get(this.url, params, function(req) {
      this.notifyControllers(-1, 'finishSlowLoad');
      if (req.status != '200') return;

      var new_tweets = JSON.parse(req.responseText).results;
      this.items = new_tweets.concat(this.items);
      this.notifyControllers();
    }.bind(this));
    this.notifyControllers(-1, 'beginSlowLoad');
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
  didUriChange: function(uri) {
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
      icon: tweet.file
    }
  }

  return {
    formatTweet: formatTweet
  }
})();

BaseProfileController = EUI.ListController({
  style: 'double_label',
  getProfileImage: function() {
    //gets profile image
    ajax.get(this.profile.profile_image_url, null, function(req) {
      if (req.status != '200') return;
      var item = this.fields[0];

      item.icon = req.responseText;
      this.updateView();
    }.bind(this));
  },
  init: function(user_id) {
    this.fields = [];
    var canShowProfile = false;

    //these two gets are for having tweets and profile data before showing them
    twitterAjax.get('1/users/show.json', {user_id: user_id}, function(req) {
      if (req.status != '200') return;
      this.profile = JSON.parse(req.responseText);

      if (canShowProfile)
        this.showProfile();
      else
        canShowProfile = true;
    }.bind(this));

    twitterAjax.get("1/statuses/user_timeline.json",
                    {user_id: user_id, count: 5}, function(req) {
      if (req.status == '200')
        this.tweets = JSON.parse(req.responseText);

      if (canShowProfile)
        this.showProfile();
      else
        canShowProfile = true;
    }.bind(this));

    this.model = new EUI.ArrayModel([]);
  },
  showProfile: function() {
    this.title = this.profile.name;

    this.fields = [
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
      this.fields.push(tweet);
    }
    //Add more tweets button
    this.fields.push({
      text: 'More tweets',
      tag: 'more-tweets',
      end: 'arrow_right',
      group: 'Tweets'
    });

    for (var i = 0; i < this.fields.length; i++)
      this.model.pushItem(this.fields[i]);
  },
  createLabel: function(text) {
    return EUI.widgets.Label({
      label: text,
      single_line: true
    });
  },
  itemAtIndex: function(index) {
    var item = this.fields[index];
    if (index == 0 && !item.icon)
      this.getProfileImage();

    if (item.tag == 'tweet' && !item.file && this.fields[0].icon)
      item.icon = this.fields[0].icon;

    return item;
  },
  selectedItemAtIndex: function(index) {
    if (this.fields[index].tag == 'more-tweets')
        this.pushController(new TimelineController(
           new TimelineModel(this.profile.id_str),
           'tweets', this.profile.screen_name));
  }
});

UserProfileController = BaseProfileController.extend({
  showProfile: function(profile) {
    this._super(profile);

    //[un]follow action
    this.fields.push({
      text: this.profile.following ? 'Unfollow' : 'Follow',
      tag: 'follow-action',
      end: 'arrow_right',
      group: 'Actions'
    });
    this.unFollowIndex = this.fields.length -1;
    this.model.pushItem(this.fields[this.unFollowIndex]);
  },
  callbackFollowUnfollow: function(req) {
    if (req.status != 200) return;

    //update profile
    this.profile = JSON.parse(req.responseText);
    this.evaluateViewChanges();
    this.fields[this.unFollowIndex].text =
      this.profile.following ? 'Unfollow' : 'Follow';
    this.updateView(this.unFollowIndex);
  },
  selectedItemAtIndex: function(index) {
    if (this.fields[index].tag == 'follow-action'){
      if (this.profile.following)
        twitterAjax.post('1/friendships/destroy.json',
                          {user_id: this.profile.id_str},
                          this.callbackFollowUnfollow.bind(this));
      else
         twitterAjax.post('1/friendships/create.json',
                          {user_id: this.profile.id_str},
                          this.callbackFollowUnfollow.bind(this));

      return;
    }

    this._super(index);
  },
});

LoggedInUserProfileController = BaseProfileController.extend({
  init: function(){
    this._super(gUserID);
    this.title = 'Me';
  },
  showProfile: function(profile) {
    this._super(profile);
    this.title = 'Me';

    //signout action
    this.fields.push({
      text: 'Sign out',
      tag: 'sign-out',
      end: 'arrow_right',
      group: 'Actions'
    });
    this.model.pushItem(this.fields[this.fields.length]);
  },
  selectedItemAtIndex: function(index) {
    if (this.fields[index].tag == 'sign-out') {
      gToken = gTokenSecret = null;

      localStorage.removeItem('gToken');
      localStorage.removeItem('gTokenSecret');
      localStorage.removeItem('gScreenName');
      localStorage.removeItem('gUserID');

      this.popController();
      return;
    }

    this._super(index);
  }
});

TimelineController = EUI.ListController({
  icon: 'go-home',
  style: 'double_label',
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
