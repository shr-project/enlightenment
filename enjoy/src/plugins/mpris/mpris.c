#include <Eina.h>
#include <EDBus.h>
#include <Ecore.h>

#include "plugin.h"
#include "song.h"

typedef struct _MPRIS_Method MPRIS_Method;
typedef struct _MPRIS_Signal MPRIS_Signal;

static int _mpris_log_domain = -1;

#ifdef CRITICAL
#undef CRITICAL
#endif
#ifdef ERR
#undef ERR
#endif
#ifdef WRN
#undef WRN
#endif
#ifdef INF
#undef INF
#endif
#ifdef DBG
#undef DBG
#endif

#define CRITICAL(...) EINA_LOG_DOM_CRIT(_mpris_log_domain, __VA_ARGS__)
#define ERR(...)      EINA_LOG_DOM_ERR(_mpris_log_domain, __VA_ARGS__)
#define WRN(...)      EINA_LOG_DOM_WARN(_mpris_log_domain, __VA_ARGS__)
#define INF(...)      EINA_LOG_DOM_INFO(_mpris_log_domain, __VA_ARGS__)
#define DBG(...)      EINA_LOG_DOM_DBG(_mpris_log_domain, __VA_ARGS__)


/*
 * Capabilities and player status values conform to the MPRIS 1.0 standard:
 * http://www.mpris.org/1.0/spec.html
 */
typedef enum {
  MPRIS_CAPABILITY_CAN_GO_NEXT = 1 << 0,
  MPRIS_CAPABILITY_CAN_GO_PREV = 1 << 1,
  MPRIS_CAPABILITY_CAN_PAUSE = 1 << 2,
  MPRIS_CAPABILITY_CAN_PLAY = 1 << 3,
  MPRIS_CAPABILITY_CAN_SEEK = 1 << 4,
  MPRIS_CAPABILITY_CAN_PROVIDE_METADATA = 1 << 5,
  MPRIS_CAPABILITY_HAS_TRACKLIST = 1 << 6
} Mpris_Capabilities;

#define APPLICATION_NAME "org.mpris.enjoy"
#define PLAYER_INTERFACE_NAME "org.freedesktop.MediaPlayer"
#define ROOT_NAME "/Root" /* should really be "/", but this doesn't work correctly :( */
#define TRACKLIST_NAME "/TrackList"
#define PLAYER_NAME "/Player"

static void _mpris_signal_player_caps_change(int caps);
static void _mpris_signal_player_status_change(int playback, int shuffle, int repeat, int endless);
static void _mpris_signal_player_track_change(const Song *song);
static void _mpris_signal_tracklist_tracklist_change(int size);

static void _mpris_append_dict_entry(EDBus_Message_Iter *array, const char *key, const char  *value_type, ...);
static EDBus_Message *_mpris_player_next(const EDBus_Service_Interface *iface, const EDBus_Message *msg);
static EDBus_Message *_mpris_player_previous(const EDBus_Service_Interface *iface, const EDBus_Message *msg);
static EDBus_Message *_mpris_player_pause(const EDBus_Service_Interface *iface, const EDBus_Message *msg);
static EDBus_Message *_mpris_player_stop(const EDBus_Service_Interface *iface, const EDBus_Message *msg);
static EDBus_Message *_mpris_player_play(const EDBus_Service_Interface *iface, const EDBus_Message *msg);
static EDBus_Message *_mpris_player_seek(const EDBus_Service_Interface *iface, const EDBus_Message *msg);
static EDBus_Message *_mpris_root_identity(const EDBus_Service_Interface *iface, const EDBus_Message *msg);
static EDBus_Message *_mpris_root_quit(const EDBus_Service_Interface *iface, const EDBus_Message *msg);
static EDBus_Message *_mpris_root_version(const EDBus_Service_Interface *iface, const EDBus_Message *msg);
static EDBus_Message *_mpris_player_caps_get(const EDBus_Service_Interface *iface, const EDBus_Message *msg);
static EDBus_Message *_mpris_player_volume_set(const EDBus_Service_Interface *iface, const EDBus_Message *msg);
static EDBus_Message *_mpris_player_volume_get(const EDBus_Service_Interface *iface, const EDBus_Message *msg);
static EDBus_Message *_mpris_player_repeat_set(const EDBus_Service_Interface *iface, const EDBus_Message *msg);
static EDBus_Message *_mpris_player_status_get(const EDBus_Service_Interface *iface, const EDBus_Message *msg);
static EDBus_Message *_mpris_player_position_set(const EDBus_Service_Interface *iface, const EDBus_Message *msg);
static EDBus_Message *_mpris_player_position_get(const EDBus_Service_Interface *iface, const EDBus_Message *msg);
static EDBus_Message *_mpris_song_metadata_reply(const EDBus_Message *msg, const Song *song);
static EDBus_Message *_mpris_player_metadata_get(const EDBus_Service_Interface *iface, const EDBus_Message *msg);
static EDBus_Message *_mpris_tracklist_current_track_get(const EDBus_Service_Interface *iface, const EDBus_Message *msg);
static EDBus_Message *_mpris_tracklist_count(const EDBus_Service_Interface *iface, const EDBus_Message *msg);
static EDBus_Message *_mpris_tracklist_metadata_get(const EDBus_Service_Interface *iface, const EDBus_Message *msg);
static EDBus_Message *_mpris_tracklist_shuffle_set(const EDBus_Service_Interface *iface, const EDBus_Message *msg);

static void _cb_dbus_request_name(void *data, const EDBus_Message *msg, EDBus_Pending *pending);

static EDBus_Connection *conn = NULL;
static EDBus_Service_Interface *root, *player, *tracklist;
static Eina_List *ev_handlers = NULL;

enum
{
   PLAYER_TRACK = 0,
   PLAYER_STATUS,
   PLAYER_CAPS
};

static const EDBus_Signal mpris_player_signals[] = {
   /* Emitted whenever a new song is played; gives the song metadata */
   [PLAYER_TRACK] = { "TrackChange",  EDBUS_ARGS({"a{sv}", ""}), 0 },
   /* Emitted whenever player's status changes */
   [PLAYER_STATUS] = { "StatusChange", EDBUS_ARGS({"(iiii)", ""}), 0 },
   /* Emitted whenever player's capabilities changes */
   [PLAYER_CAPS] = { "CapsChange", EDBUS_ARGS({"i", ""}), 0 },
   {  }
};

enum
{
   TRACK_LIST = 0,
};

static const EDBus_Signal mpris_tracklist_signals[] = {
   /* Emitted whenever the tracklist changes; gives the number of items */
   [TRACK_LIST] = { "TrackListChange", EDBUS_ARGS({"i", ""}), 0 },
   {  }
};

static const EDBus_Method mpris_root_methods[] = {
   /* Returns a string representing the player name */
   {
    "Identity", NULL, EDBUS_ARGS({"s", "name"}), _mpris_root_identity, 0
   },
   /* Quits the player */
   { "Quit", NULL, NULL, _mpris_root_quit, 0 },
   /* Returns a tuple containing the version of MPRIS protocol implemented */
   {
    "MprisVersion", NULL, EDBUS_ARGS({"(qq)", ""}), _mpris_root_version, 0
   },
   { }
};

static const EDBus_Method mpris_player_methods[] = {
   /* Goes to the next song */
   {
    "Next", NULL, NULL, _mpris_player_next, 0
   },
   /* Goes to the previous song */
   {
    "Prev", NULL, NULL, _mpris_player_previous, 0
   },
   /* Pauses the song */
   {
    "Pause", NULL, NULL, _mpris_player_pause, 0
   },
   /* Stops the song */
   {
    "Stop", NULL, NULL, _mpris_player_stop, 0
   },
   /* If playing, rewind to the beginning of the current track; else, start playing */
   {
    "Play", NULL, NULL, _mpris_player_play, 0
   },
   /* Seek the current song by given miliseconds */
   {
    "Seek", EDBUS_ARGS({"x", "time"}), NULL, _mpris_player_seek, 0
   },
   /* Toggle the current track repeat */
   {
    "Repeat", EDBUS_ARGS({"b", ""}), NULL, _mpris_player_repeat_set, 0
   },
   /* Return the status of the media player */
   {
    "GetStatus", NULL, EDBUS_ARGS({"(iiii)", ""}), _mpris_player_status_get, 0
   },
   /* Gets all the metadata for the currently played element */
   {
    "GetMetadata", NULL, EDBUS_ARGS({"a{sv}", "data"}),
    _mpris_player_metadata_get, 0
   },
   /* Returns the media player's current capabilities */
   {
    "GetCaps", NULL, EDBUS_ARGS({"i", ""}), _mpris_player_caps_get, 0
   },
   /* Sets the volume */
   {
    "VolumeSet", EDBUS_ARGS({"i", ""}), NULL, _mpris_player_volume_set, 0
   },
   /* Gets the current volume */
   {
    "VolumeGet", NULL, EDBUS_ARGS({"i", ""}), _mpris_player_volume_get, 0
   },
   /* Sets the playing position (in ms) */
   {
    "PositionSet", EDBUS_ARGS({"i", ""}), NULL, _mpris_player_position_set, 0
   },
   /* Gets the playing position (in ms) */
   {
    "PositionGet", NULL, EDBUS_ARGS({"i", ""}), _mpris_player_position_get, 0
   },
   { }
};

static const EDBus_Method mpris_tracklist_methods[] = {
   /* Gives all the metadata available at the given position in the track list */
   {
    "GetMetadata", EDBUS_ARGS({"i", ""}), EDBUS_ARGS({"a{sv}", ""}),
    _mpris_tracklist_metadata_get, 0
   },
   /* Returns the position of the current URI in the track list */
   {
    "GetCurrentTrack", NULL, EDBUS_ARGS({"i", ""}),
    _mpris_tracklist_current_track_get, 0
   },
   /* Returns the number of elements in the track list */
   {
    "GetLength", NULL, EDBUS_ARGS({"i", ""}), _mpris_tracklist_count, 0
   },
   /* Appends an URI to the track list */
   /*{ "AddTrack", EDBUS_ARGS({"sb", ""}), EDBUS_ARGS({"i", ""}), NULL, 0 },*/
   /* Removes an URL from the track list */
   /*{ "DelTrack", EDBUS_ARGS({"i", ""}), NULL, NULL, 0 },*/
   /* Toggle playlist loop */
   /*{ "SetLoop", EDBUS_ARGS({"b", ""}), NULL, NULL, 0 },*/
   /* Toggle playlist shuffle/random */
   {
    "SetRandom", EDBUS_ARGS({"b", ""}), NULL, _mpris_tracklist_shuffle_set, 0
   },
   { }
};

static int
_caps_to_mpris_bits(const Enjoy_Player_Caps caps)
{
   int bits = 0;
   if (caps.can_go_next) bits |= MPRIS_CAPABILITY_CAN_GO_NEXT;
   if (caps.can_go_prev) bits |= MPRIS_CAPABILITY_CAN_GO_PREV;
   if (caps.can_pause) bits |= MPRIS_CAPABILITY_CAN_PAUSE;
   if (caps.can_play) bits |= MPRIS_CAPABILITY_CAN_PLAY;
   if (caps.can_seek) bits |= MPRIS_CAPABILITY_CAN_SEEK;
   if (caps.can_provide_metadata) bits |= MPRIS_CAPABILITY_CAN_PROVIDE_METADATA;
   if (caps.has_tracklist) bits |= MPRIS_CAPABILITY_HAS_TRACKLIST;
   return bits;
}

static Eina_Bool
_cb_player_caps_change(void *data __UNUSED__, int type __UNUSED__, void *event __UNUSED__)
{
   Enjoy_Player_Caps caps = enjoy_player_caps_get();
   int bits = _caps_to_mpris_bits(caps);
   _mpris_signal_player_caps_change(bits);
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_cb_player_status_change(void *data __UNUSED__, int type __UNUSED__, void *event __UNUSED__)
{
   Enjoy_Player_Status status = enjoy_player_status_get();
   _mpris_signal_player_status_change
     (status.playback, status.shuffle, status.repeat, status.endless);
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_cb_player_track_change(void *data __UNUSED__, int type __UNUSED__, void *event __UNUSED__)
{
   _mpris_signal_player_track_change(enjoy_song_current_get());
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_cb_player_tracklist_change(void *data __UNUSED__, int type __UNUSED__, void *event __UNUSED__)
{
   _mpris_signal_tracklist_tracklist_change(enjoy_playlist_count());
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
mpris_enable(Enjoy_Plugin *p __UNUSED__)
{
#define EV_HANDLER(ev, func, data)              \
   ev_handlers = eina_list_append               \
     (ev_handlers, ecore_event_handler_add(ev, func, data))

   EV_HANDLER(ENJOY_EVENT_PLAYER_CAPS_CHANGE, _cb_player_caps_change, NULL);
   EV_HANDLER(ENJOY_EVENT_PLAYER_STATUS_CHANGE, _cb_player_status_change, NULL);
   EV_HANDLER(ENJOY_EVENT_PLAYER_TRACK_CHANGE, _cb_player_track_change, NULL);
   EV_HANDLER(ENJOY_EVENT_TRACKLIST_TRACKLIST_CHANGE,
              _cb_player_tracklist_change, NULL);
#undef EV_HANDLER

   edbus_name_request(conn, APPLICATION_NAME,
                      EDBUS_NAME_REQUEST_FLAG_DO_NOT_QUEUE,
                      _cb_dbus_request_name, NULL);
   return EINA_TRUE;
}

static Eina_Bool
mpris_disable(Enjoy_Plugin *p __UNUSED__)
{
   Ecore_Event_Handler *eh;

   if (root)
     {
        edbus_service_object_unregister(root);
        edbus_service_object_unregister(tracklist);
        edbus_service_object_unregister(player);
        root = NULL;
        tracklist = NULL;
        player = NULL;
     }

   EINA_LIST_FREE(ev_handlers, eh)
     ecore_event_handler_del(eh);

   return EINA_TRUE;
}

static const Enjoy_Plugin_Api api = {
  ENJOY_PLUGIN_API_VERSION,
  mpris_enable,
  mpris_disable
};

static Eina_Bool
mpris_init(void)
{
   if (_mpris_log_domain < 0)
     {
        _mpris_log_domain = eina_log_domain_register
          ("enjoy-mpris", EINA_COLOR_LIGHTCYAN);
        if (_mpris_log_domain < 0)
          {
             EINA_LOG_CRIT("Could not register log domain 'enjoy-mpris'");
             return EINA_FALSE;
          }
     }

   if (!ENJOY_ABI_CHECK())
     {
        ERR("ABI versions differ: enjoy=%u, mpris=%u",
            enjoy_abi_version(), ENJOY_ABI_VERSION);
        goto error;
     }

   if (conn) return EINA_TRUE;

   edbus_init();
   conn = edbus_connection_get(EDBUS_CONNECTION_TYPE_SESSION);
   if (!conn)
     {
        ERR("Could not get DBus session bus");
        goto error;
     }

   enjoy_plugin_register("listener/mpris", &api, ENJOY_PLUGIN_PRIORITY_HIGH);

   return EINA_TRUE;

 error:
   eina_log_domain_unregister(_mpris_log_domain);
   _mpris_log_domain = -1;
   return EINA_FALSE;
}

void
mpris_shutdown(void)
{
   if (!conn) return;

   edbus_connection_unref(conn);
   edbus_shutdown();
   conn = NULL;

   if (_mpris_log_domain >= 0)
     {
        eina_log_domain_unregister(_mpris_log_domain);
        _mpris_log_domain = -1;
     }
}

static const EDBus_Service_Interface_Desc root_desc = {
   PLAYER_INTERFACE_NAME, mpris_root_methods
};

static const EDBus_Service_Interface_Desc player_desc = {
   PLAYER_INTERFACE_NAME, mpris_player_methods, mpris_player_signals
};

static const EDBus_Service_Interface_Desc tracklist_desc = {
   PLAYER_INTERFACE_NAME, mpris_tracklist_methods, mpris_tracklist_signals
};

static void
_cb_dbus_request_name(void *data __UNUSED__, const EDBus_Message *msg, EDBus_Pending *pending __UNUSED__)
{
   const char *error_name, *error_txt;
   unsigned flag;

   if (edbus_message_error_get(msg, &error_name, &error_txt))
     {
        ERR("Error %s %s", error_name, error_txt);
        return;
     }

   if (!edbus_message_arguments_get(msg, "u", &flag))
     {
        ERR("Error getting arguments.");
        return;
     }

   if (flag != EDBUS_NAME_REQUEST_REPLY_PRIMARY_OWNER)
     {
        ERR("Bus name in use by another application.");
        return;
     }

   root = edbus_service_interface_register(conn, ROOT_NAME, &root_desc);
   player = edbus_service_interface_register(conn, PLAYER_NAME, &player_desc);
   tracklist = edbus_service_interface_register(conn, TRACKLIST_NAME,
                                                &tracklist_desc);
}

static void
_mpris_append_dict_entry(EDBus_Message_Iter *array, const char *key,
                         const char  *value_type, ...)
{
   EDBus_Message_Iter *dict, *val;
   va_list ap;

   va_start(ap, value_type);
   edbus_message_iter_arguments_set(array, "{sv}", &dict);
   edbus_message_iter_basic_append(dict, 's', key);
   val = edbus_message_iter_container_new(dict, 'v', value_type);
   edbus_message_iter_arguments_vset(val, value_type, ap);
   edbus_message_iter_container_close(dict, val);
   edbus_message_iter_container_close(array, dict);
   va_end(ap);
}

static void
_mpris_message_fill_song_metadata(EDBus_Message *msg, const Song *song)
{
   EDBus_Message_Iter *array, *main_iter;

   if (!song) return;

   /*
     Other possible metadata:
     location s		time u
     mtime u		comment s
     rating u		year u
     date u		arturl s
     genre s		mpris:length u
     trackno s
   */

   main_iter = edbus_message_iter_get(msg);
   edbus_message_iter_arguments_set(main_iter, "a{sv}", &array);

   if (song->title)
     _mpris_append_dict_entry(array, "title", "s", song->title);
   if (song->flags.fetched_album && song->album)
     _mpris_append_dict_entry(array, "album", "s", song->album);
   if (song->flags.fetched_artist && song->artist)
     _mpris_append_dict_entry(array, "artist", "s", song->artist);
   if (song->flags.fetched_genre && song->genre)
     _mpris_append_dict_entry(array, "genre", "s", song->genre);
   _mpris_append_dict_entry(array, "rating", "u", song->rating);
   _mpris_append_dict_entry(array, "length", "u", song->length);
   _mpris_append_dict_entry(array, "enjoy:playcount", "i", song->playcnt);
   _mpris_append_dict_entry(array, "enjoy:filesize", "i", song->size);

   edbus_message_iter_container_close(main_iter, array);
}

void
_mpris_signal_player_caps_change(int caps)
{
   static int old_caps = 0;
   if (caps != old_caps)
     {
        int32_t caps32 = caps;
        edbus_service_signal_emit(player, PLAYER_CAPS, caps32);
        old_caps = caps;
     }
}

static void
_mpris_signal_player_status_change(int playback, int shuffle, int repeat, int endless)
{
   EDBus_Message *sig;
   EDBus_Message_Iter *st, *main_iter;
   static int old_playback = 0, old_shuffle = 0, old_repeat = 0, old_endless = 0;

   if (old_playback == playback && old_shuffle == shuffle &&
       old_repeat == repeat && old_endless == endless) return;
   old_playback = playback;
   old_shuffle = shuffle;
   old_repeat = repeat;
   old_endless = endless;

   sig = edbus_service_signal_new(player, PLAYER_STATUS);
   if (!sig) return;

   main_iter = edbus_message_iter_get(sig);
   edbus_message_iter_arguments_set(main_iter, "(iiii)", &st);
   edbus_message_iter_basic_append(st, 'i', playback);
   edbus_message_iter_basic_append(st, 'i', shuffle);
   edbus_message_iter_basic_append(st, 'i', repeat);
   edbus_message_iter_basic_append(st, 'i', endless);
   edbus_message_iter_container_close(main_iter, st);

   edbus_service_signal_send(player, sig);
   edbus_message_unref(sig);
}

static void
_mpris_signal_player_track_change(const Song *song)
{
   static const void *old_song = NULL;
   if (old_song != song)
     {
        EDBus_Message *sig = edbus_service_signal_new(player, PLAYER_TRACK);
        if (!sig) return;
        _mpris_message_fill_song_metadata(sig, song);
        edbus_service_signal_send(player, sig);
        edbus_message_unref(sig);
        old_song = song;
     }
}

static void
_mpris_signal_tracklist_tracklist_change(int size)
{
   int32_t size32 = size;
   edbus_service_signal_emit(tracklist, TRACK_LIST, size32);
}

static EDBus_Message *
_mpris_player_next(const EDBus_Service_Interface *iface __UNUSED__, const EDBus_Message *msg)
{
   enjoy_control_next();
   return edbus_message_method_return_new(msg);
}

static EDBus_Message *
_mpris_player_previous(const EDBus_Service_Interface *iface __UNUSED__, const EDBus_Message *msg)
{
   enjoy_control_previous();
   return edbus_message_method_return_new(msg);
}

static EDBus_Message *
_mpris_player_pause(const EDBus_Service_Interface *iface __UNUSED__, const EDBus_Message *msg)
{
   enjoy_control_pause();
   return edbus_message_method_return_new(msg);
}

static EDBus_Message *
_mpris_player_stop(const EDBus_Service_Interface *iface __UNUSED__, const EDBus_Message *msg)
{
   enjoy_control_stop();
   return edbus_message_method_return_new(msg);
}

static EDBus_Message *
_mpris_player_play(const EDBus_Service_Interface *iface __UNUSED__, const EDBus_Message *msg)
{
   Enjoy_Player_Status status = enjoy_player_status_get();
   if (!status.playback)
     enjoy_position_set(0);
   enjoy_control_play();
   return edbus_message_method_return_new(msg);
}

static EDBus_Message *
_mpris_player_seek(const EDBus_Service_Interface *iface __UNUSED__, const EDBus_Message *msg)
{
   int64_t position;
   if (!edbus_message_arguments_get(msg, "x", &position))
     goto end;
   enjoy_control_seek(position);
end:
   return edbus_message_method_return_new(msg);
}

static EDBus_Message *
_mpris_root_identity(const EDBus_Service_Interface *iface __UNUSED__, const EDBus_Message *msg)
{
   const char *identity = PACKAGE_STRING;
   EDBus_Message *reply = edbus_message_method_return_new(msg);
   edbus_message_arguments_set(reply, "s", identity);
   return reply;
}

static EDBus_Message *
_mpris_root_quit(const EDBus_Service_Interface *iface __UNUSED__, const EDBus_Message *msg)
{
   enjoy_quit();
   return edbus_message_method_return_new(msg);
}

static EDBus_Message *
_mpris_root_version(const EDBus_Service_Interface *iface __UNUSED__, const EDBus_Message *msg)
{
   EDBus_Message *reply = edbus_message_method_return_new(msg);
   EDBus_Message_Iter *main_iter, *s;
   uint16_t v1 = 1, v2 = 0;

   main_iter = edbus_message_iter_get(reply);
   edbus_message_iter_arguments_set(main_iter, "(qq)", &s);
   edbus_message_iter_arguments_set(s, "qq", v1, v2);
   edbus_message_iter_container_close(main_iter, s);
   return reply;
}

static EDBus_Message *
_mpris_player_caps_get(const EDBus_Service_Interface *iface __UNUSED__, const EDBus_Message *msg)
{
   EDBus_Message *reply = edbus_message_method_return_new(msg);
   int32_t bits = _caps_to_mpris_bits(enjoy_player_caps_get());
   edbus_message_arguments_set(reply, "i", bits);
   return reply;
}

static EDBus_Message *
_mpris_player_volume_set(const EDBus_Service_Interface *iface __UNUSED__, const EDBus_Message *msg)
{
   int volume;

   if (!edbus_message_arguments_get(msg, "i", &volume))
     goto end;
   if (volume > 100)
     volume = 100;
   else if (volume < 0)
     volume = 0;
   enjoy_volume_set(volume);
end:
   return edbus_message_method_return_new(msg);
}

static EDBus_Message *
_mpris_player_volume_get(const EDBus_Service_Interface *iface __UNUSED__, const EDBus_Message *msg)
{
   EDBus_Message *reply = edbus_message_method_return_new(msg);
   int32_t vol = enjoy_volume_get();
   edbus_message_arguments_set(reply, "i", vol);
   return reply;
}

static EDBus_Message *
_mpris_player_repeat_set(const EDBus_Service_Interface *iface __UNUSED__, const EDBus_Message *msg)
{
   Eina_Bool repeat;
   if (!edbus_message_arguments_get(msg, "b", &repeat))
     goto end;
   enjoy_control_loop_set(repeat);
end:
   return edbus_message_method_return_new(msg);
}

static EDBus_Message *
_mpris_player_status_get(const EDBus_Service_Interface *iface __UNUSED__, const EDBus_Message *msg)
{
   EDBus_Message *reply = edbus_message_method_return_new(msg);
   Enjoy_Player_Status status = enjoy_player_status_get();
   EDBus_Message_Iter *main_iter, *st;
   int32_t p, s, r, e;

   p = status.playback;
   s = status.shuffle;
   r = status.repeat;
   e = status.endless;

   main_iter = edbus_message_iter_get(reply);
   edbus_message_iter_arguments_set(main_iter, "(iiii)", &st);
   edbus_message_iter_arguments_set(st, "iiii", p, s, r, e);
   edbus_message_iter_container_close(main_iter, st);

   return reply;
}

static EDBus_Message *
_mpris_player_position_set(const EDBus_Service_Interface *iface __UNUSED__, const EDBus_Message *msg)
{
   int position;
   if (!edbus_message_arguments_get(msg, "i", &position))
     goto end;
   enjoy_position_set(position);
end:
   return edbus_message_method_return_new(msg);
}

static EDBus_Message *
_mpris_player_position_get(const EDBus_Service_Interface *iface __UNUSED__, const EDBus_Message *msg)
{
   EDBus_Message *reply = edbus_message_method_return_new(msg);
   int32_t pos = enjoy_position_get();
   edbus_message_arguments_set(reply, "i", pos);
   return reply;
}

static EDBus_Message *
_mpris_song_metadata_reply(const EDBus_Message *msg, const Song *song)
{
   EDBus_Message *reply = edbus_message_method_return_new(msg);
   _mpris_message_fill_song_metadata(reply, song);
   return reply;
}

static EDBus_Message *
_mpris_player_metadata_get(const EDBus_Service_Interface *iface __UNUSED__, const EDBus_Message *msg)
{
   return _mpris_song_metadata_reply(msg, enjoy_song_current_get());
}

static EDBus_Message *
_mpris_tracklist_current_track_get(const EDBus_Service_Interface *iface __UNUSED__, const EDBus_Message *msg)
{
   EDBus_Message *reply = edbus_message_method_return_new(msg);
   int32_t pos = enjoy_playlist_current_position_get();
   edbus_message_arguments_set(reply, "i", pos);
   return reply;
}

static EDBus_Message *
_mpris_tracklist_count(const EDBus_Service_Interface *iface __UNUSED__, const EDBus_Message *msg)
{
   EDBus_Message *reply = edbus_message_method_return_new(msg);
   int32_t count = enjoy_playlist_count();
   edbus_message_arguments_set(reply, "i", count);
   return reply;
}

static EDBus_Message *
_mpris_tracklist_metadata_get(const EDBus_Service_Interface *iface __UNUSED__, const EDBus_Message *msg)
{
   EDBus_Message *reply;
   const Song *song;
   int position;
   if (!edbus_message_arguments_get(msg, "i", &position))
     return NULL;
   song = enjoy_playlist_song_position_get(position);
   reply = _mpris_song_metadata_reply(msg, song);
   return reply;
}

static EDBus_Message *
_mpris_tracklist_shuffle_set(const EDBus_Service_Interface *iface __UNUSED__, const EDBus_Message *msg)
{
   Eina_Bool param;
   if (!edbus_message_arguments_get(msg, "b", &param))
     goto end;
   enjoy_control_shuffle_set(param);
end:
   return edbus_message_method_return_new(msg);
}

EINA_MODULE_INIT(mpris_init);
EINA_MODULE_SHUTDOWN(mpris_shutdown);
