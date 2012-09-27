var EUI = require('eui');
var ajax = require('ajax');

const KEY = "sbrubles";
const URL = "api.rottentomatoes.com/api/public/v1.0/movies.json";

RottenTomatoesModel = EUI.Model({
  init: function(url) {
    this.url = this.url || url;
    this.array = [];
  },
  refresh: function() {
    ajax.ajax(this.url, {
      blockUI: true,
      data: {
        apikey: KEY,
        q: this.query
      },
      onSuccess: function(request) {
        this.array = JSON.parse(request.responseText).movies || [];
        this.notifyControllers();
      }.bind(this)
    });
  },
  getPoster: function(index) {
    var movie = this.array[index];

    if (movie.poster)
      return;

    movie.request = ajax.get(movie.posters.original, null, function(request) {
      movie.file = movie.poster = request.responseText;
      delete movie.request;
      this.notifyControllers();
    }.bind(this, movie));
  },
  length: function() {
    return this.array.length;
  },
  setFilter: function(terms) {
    if (!terms) return;
    this.query = terms.toLowerCase();
    this.event = clearInterval(this.event);

    this.event = setTimeout(function(){
      this.array = [];
      this.previousQuery = this.query;
      this.refresh();
    }.bind(this), 200);
  },
  itemAtIndex: function(index) {
    var item = this.array[index];

    if (item && !item.file && !item.request) {
     item.request =  ajax.ajax(item.posters.thumbnail, {
        onSuccess: function(request) {
          item.file = request.responseText;
          this.notifyControllers();
        }.bind(this)
      });
    }

    return item;
  }
});

MovieController = EUI.FormController({
  init: function(model, index) {
    this.model = model;
    this.index = index;
    this.fields = [];
    this.model.getPoster(index);

    var movie = this.model.itemAtIndex(this.index);

    var attributes = [
      {tag: 'title', label: 'Title'},
      {tag: 'year', label: 'Year'},
      {tag: 'runtime', label: 'Duration'},
      {tag: 'synopsis', label: 'Synopsis'},
      {tag: 'mpaa_rating', label: 'Rating'},
    ];

    /* Add title, year, duration, synopsis and movie rating */
    for (var i in attributes) {
      this.fields.push([
        EUI.widgets.Label({
          label: '<b>' + attributes[i]['label'] + ':</b> ' + movie[attributes[i]['tag']],
          single_line: true,
          expand: 'horizontal',
        })
      ], '>');
    }

    /* Add critics and audience scores */
    this.fields.push([
      EUI.widgets.Label({
        label: '<b>Critics Score:</b>: ' + movie['ratings']['critics_score'],
        single_line: true,
        expand: 'horizontal',
      }),
      EUI.widgets.Label({
        label: '<b>Audience Score:</b>: ' + movie['ratings']['audience_score'],
        single_line: true,
        expand: 'horizontal',
      })]);

    /* Add the movie poster */
    this.fields.push([
      EUI.widgets.Photocam({zoom_mode: 'auto-fit', field: 'file'}), '>'
    ]);

    /* Add critics */
    this.fields.push([
      EUI.widgets.Label({text: '<b>Critics</b>',
                         single_line: true,
                         expand: 'horizontal'
                        }), '>'
    ]);
    this.fields.push([
      EUI.widgets.Entry({text: '<i>' + movie.critics_consensus + '</i>',
                         expand: 'horizontal'
                        }), '>'
    ]);

    /* Add cast */
    this.fields.push([EUI.widgets.Label({
      text: '<b>Cast</b>',
      single_line: true,
      expand: 'horizontal'
    }), '>']);
    for (var i in movie.abridged_cast) {
      var actor = movie.abridged_cast[i].name;
      var characters = movie.abridged_cast[i].characters || [];

      this.fields.push([
        EUI.widgets.Label({
          text: actor,
          single_line: true,
          expand: 'horizontal'
        }),
        EUI.widgets.Label({
          text: '<i>' + characters.join(' ') + '</i>',
          single_line: true,
          expand: 'horizontal'
        })
      ]);
    }
  }
});

RottenTomatoesController = EUI.ListController({
  title: 'Rotten Tomatoes',
  model: new RottenTomatoesModel(URL),

  itemAtIndex: function(index) {
    var item = this.model.itemAtIndex(index);
    return item && {text: item.title, icon: item.file};
  },
  selectedItemAtIndex: function(index) {
    this.pushController(new MovieController(this.model, index));
  },
  search: function(text) {
    if (text)
      this.model.setFilter(text);
  }
});

if (KEY)
  EUI.app(new RottenTomatoesController());
else
  print("Get an API key at http://developer.rottentomatoes.com");
