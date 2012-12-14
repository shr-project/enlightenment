#ifndef __ETROPHY_H__
#define __ETROPHY_H__

/**
 * @mainpage Etrophy Library Documentation
 *
 * @version 0.5.0
 * @date 2012
 *
 * @section intro What is Etrophy ?
 *
 * Etrophy is a library that manages scores, trophies and unlockables. It will
 * store them and provide views to display them. Could be used by games based
 * on EFL.
 *
 * For a better reference, check the following groups:
 * @li @ref Init
 * @li @ref Gamescore
 * @li @ref Score
 * @li @ref Trophy
 * @li @ref Lock
 * @li @ref View
 *
 * Please see the @ref authors page for contact details.
 */

/**
 *
 * @page authors Authors
 *
 * @author Bruno Dilly <bdilly@@profusion.mobi>
 * @author Mike Blumenkrantz <michael.blumenkrantz@@gmail.com>
 *
 * Please contact <enlightenment-devel@lists.sourceforge.net> to get in
 * contact with the developers and maintainers.
 *
 */

#include <Eina.h>
#include <Eet.h>
#include <Evas.h>

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef EFL_ETROPHY_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif /* ! DLL_EXPORT */
# else
#  define EAPI __declspec(dllimport)
# endif /* ! EFL_ETROPHY_BUILD */
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EAPI __attribute__ ((visibility("default")))
#  else
#   define EAPI
#  endif
# else
#  define EAPI
# endif
#endif /* ! _WIN32 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file
 * @brief These routines are used for Etrophy library interaction.
 */

/**
 * @brief Init / shutdown functions.
 * @defgroup Init  Init / Shutdown
 *
 * @{
 *
 * Functions of obligatory usage, handling proper initialization
 * and shutdown routines.
 *
 * Before the usage of any other function, Etrophy should be properly
 * initialized with @ref etrophy_init() and the last call to Etrophy's
 * functions should be @ref etrophy_shutdown(), so everything will
 * be correctly freed.
 *
 * Etrophy logs everything with Eina Log, using the "etrophy" log domain.
 *
 */

EAPI int etrophy_init(void);
EAPI int etrophy_shutdown(void);

/**
 * @}
 */

/**
 * @brief Gamescore API available to add a gamescore for your application.
 * @defgroup Gamescore Gamescore
 *
 * @{
 *
 * Gamescore is the object that will integrate all the components required
 * for the application, like trophies, scores and locks. It's mandatory
 * to have one created gamescore before adding anything else, so generally
 * an application should start by handling it.
 *
 * Example:
 * @code
 * Etrophy_Gamescore *gamescore;
 * gamescore = etrophy_gamescore_load("gamename");
 * if (!gamescore)
 *   {
 *      gamescore = etrophy_gamescore_new("gamename");
 *      // Functions populating the generated gamescore like a bunch of
 *      // @ref etrophy_lock_new(), @ref etrophy_gamescore_lock_add()
 *   }
 * @endcode
 *
 * Gamescore can be created with @ref etrophy_gamescore_new(), or loaded
 * with @ref etrophy_gamescore_load(), @ref etrophy_gamescore_path_load().
 * @ref etrophy_gamescore_edd_get() can be used to get the Eet data descriptor.
 *
 * The gamescore must to be saved with @ref etrophy_gamescore_save() or changes
 * will be lost. It can be cleared with @ref etrophy_gamescore_clear().
 *
 * After usage it should be freed with @ref etrophy_gamescore_free().
 *
 */

typedef struct _Etrophy_Gamescore Etrophy_Gamescore;

EAPI void etrophy_gamescore_clear(Etrophy_Gamescore *gamescore);
EAPI Etrophy_Gamescore *etrophy_gamescore_new(const char *gamename);
EAPI void etrophy_gamescore_free(Etrophy_Gamescore *gamescore);
EAPI Eet_Data_Descriptor *etrophy_gamescore_edd_get(void);
EAPI Etrophy_Gamescore *etrophy_gamescore_path_load(const char *filename);
EAPI Etrophy_Gamescore *etrophy_gamescore_load(const char *gamename);
EAPI Eina_Bool etrophy_gamescore_save(Etrophy_Gamescore *gamescore, const char *filename);

/**
 * @}
 */

/**
 * @brief Functions to manipulate player's scores.
 * @defgroup Score Score
 *
 * @{
 *
 * Scores consists of the player name, amount of points earned, and date
 * and time. It should be added to the specific level. So basically,
 * a gamescore has many levels, and each level many scores saved.
 * So a level just have a name and a list of scores.
 *
 * These two objects are handled by @ref Etrophy_Score and @ref Etrophy_Level.
 *
 * Levels can be created by two ways:
 * @li Creating a level inside a gamescore with
 * @ref etrophy_gamescore_level_add().
 * @li Creating a level with @ref etrophy_level_new() and adding it to
 * gamescore with @ref etrophy_gamescore_level_add();
 *
 * The list of levels in the gamescore can be get with
 * @ref etrophy_gamescore_levels_list_get() and can be cleared with
 * @ref etrophy_gamescore_levels_list_clear(). A specific level can be get
 * with @ref etrophy_gamescore_level_get().
 *
 * Scores can be created using @ref etrophy_score_new() and added to a
 * level with @ref etrophy_gamescore_level_score_add() or directly with
 * @ref etrophy_level_score_add(). A list of scores can be get
 * with @ref etrophy_level_scores_list_get(). But a couple of functions
 * can be very useful:
 * @li @ref etrophy_gamescore_level_hi_score_get() to get the highest score
 * in a specific level and
 * @li @ref etrophy_gamescore_level_hi_score_get() to get the lowest one
 * (useful for games based on time).
 *
 * Score properties can be accessed with:
 * @li @ref etrophy_score_player_name_get()
 * @li @ref etrophy_score_score_get()
 * @li @ref etrophy_score_date_get()
 *
 */

typedef struct _Etrophy_Level Etrophy_Level;
typedef struct _Etrophy_Score Etrophy_Score;

EAPI Etrophy_Level *etrophy_level_new(const char *name);
EAPI void etrophy_level_free(Etrophy_Level *level);
EAPI const char *etrophy_level_name_get(const Etrophy_Level *level);
EAPI const Eina_List *etrophy_level_scores_list_get(const Etrophy_Level *level);
EAPI void etrophy_level_scores_list_clear(Etrophy_Level *level);
EAPI void etrophy_gamescore_level_add(Etrophy_Gamescore *gamescore, Etrophy_Level *level);
EAPI void etrophy_gamescore_level_del(Etrophy_Gamescore *gamescore, Etrophy_Level *level);
EAPI Etrophy_Level *etrophy_gamescore_level_get(Etrophy_Gamescore *gamescore, const char *name);
EAPI const Eina_List *etrophy_gamescore_levels_list_get(const Etrophy_Gamescore *gamescore);
EAPI void etrophy_gamescore_levels_list_clear(Etrophy_Gamescore *gamescore);
EAPI void etrophy_level_score_add(Etrophy_Level *level, Etrophy_Score *escore);
EAPI void etrophy_level_score_del(Etrophy_Level *level, Etrophy_Score *escore);
EAPI Etrophy_Score *etrophy_gamescore_level_score_add(Etrophy_Gamescore *gamescore, const char *level_name, const char *player_name, int score);
EAPI Etrophy_Score *etrophy_score_new(const char *player_name, int score);
EAPI void  etrophy_score_free(Etrophy_Score *escore);
EAPI const char *etrophy_score_player_name_get(const Etrophy_Score *escore);
EAPI int etrophy_score_score_get(const Etrophy_Score *escore);
EAPI unsigned int etrophy_score_date_get(const Etrophy_Score *escore);
EAPI int etrophy_gamescore_level_hi_score_get(const Etrophy_Gamescore *gamescore, const char *level_name);
EAPI int etrophy_gamescore_level_low_score_get(const Etrophy_Gamescore *gamescore, const char *level_name);

/**
 * @}
 */

/**
 * @brief Functions to manipulate player's trophies.
 * @defgroup Trophy Trophy
 *
 * @{
 *
 * A trophy represents a task a user achieved. It can be visible or hidden,
 * and starts locked and only unlocked after the player reaches
 * its goal. For that, a trophy consists of a name, description, visibility
 * the goal, and current points on this task.
 *
 * A new trophy can be created with @ref etrophy_trophy_new() and should
 * be added to the gamescore with @ref etrophy_gamescore_trophy_add().
 * It is handled by @ref Etrophy_Trophy.
 *
 * It's counter can be directly set with @ref etrophy_trophy_counter_set()
 * or incremented using @ref etrophy_trophy_counter_increment(). Current
 * points and goal can be get using @ref etrophy_trophy_goal_get().
 * @ref etrophy_trophy_earned_get() returns if the trophy was achieved
 * or not.
 *
 * Other attributes can be read with:
 * @li @ref etrophy_trophy_name_get()
 * @li @ref etrophy_trophy_description_get()
 * @li @ref etrophy_trophy_visibility_get()
 * @li @ref etrophy_trophy_date_get()
 *
 * The trophies list can be get with
 * @ref etrophy_gamescore_trophies_list_get() and cleared using
 * @ref etrophy_gamescore_trophies_list_clear().
 */

typedef struct _Etrophy_Trophy Etrophy_Trophy;
typedef enum
{
   ETROPHY_TROPHY_STATE_HIDDEN = 0,
   ETROPHY_TROPHY_STATE_VISIBLE,
   ETROPHY_TROPHY_STATE_LAST_VALUE
} Etrophy_Trophy_Visibility;

EAPI Etrophy_Trophy *etrophy_trophy_new(const char *name, const char *description, Etrophy_Trophy_Visibility visibility, unsigned int goal);
EAPI void etrophy_trophy_free(Etrophy_Trophy *trophy);
EAPI const char *etrophy_trophy_name_get(const Etrophy_Trophy *trophy);
EAPI const char *etrophy_trophy_description_get(const Etrophy_Trophy *trophy);
EAPI Etrophy_Trophy_Visibility etrophy_trophy_visibility_get(const Etrophy_Trophy *trophy);
EAPI void etrophy_trophy_goal_get(const Etrophy_Trophy *trophy, unsigned int *goal, unsigned int *counter);
EAPI void etrophy_trophy_counter_set(Etrophy_Trophy *trophy, unsigned int value);
EAPI void etrophy_trophy_counter_increment(Etrophy_Trophy *trophy, unsigned int value);
EAPI Eina_Bool etrophy_trophy_earned_get(const Etrophy_Trophy *trophy);
EAPI unsigned int etrophy_trophy_date_get(const Etrophy_Trophy *trophy);

EAPI void etrophy_gamescore_trophy_add(Etrophy_Gamescore *gamescore, Etrophy_Trophy *trophy);
EAPI void etrophy_gamescore_trophy_del(Etrophy_Gamescore *gamescore, Etrophy_Trophy *trophy);
EAPI Etrophy_Trophy *etrophy_gamescore_trophy_get(Etrophy_Gamescore *gamescore, const char *name);
EAPI const Eina_List *etrophy_gamescore_trophies_list_get(const Etrophy_Gamescore *gamescore);
EAPI void etrophy_gamescore_trophies_list_clear(Etrophy_Gamescore *gamescore);

/**
 * @}
 */

/**
 * @brief Functions to handle locks.
 * @defgroup Lock Lock
 *
 * @{
 *
 * A lock is a very simple and generic concept, generally used for games
 * regarding stuff that should be accesible only after the user does a
 * specific action. For example: a gun is locked until the player kills the
 * first boss; or the "hard" level is unlocked only when the "easy" one is
 * concluded with a high score.
 *
 * It consists of a name, date of update, and its state: locked or unlocked.
 *
 * It is handled by @ref Etrophy_Lock and should be created with
 * @ref etrophy_lock_new() and added to the gamescore using
 * @ref etrophy_gamescore_lock_add().
 *
 * A list of locks can be get with @ref etrophy_gamescore_locks_list_get()
 * and cleared with @ref etrophy_gamescore_locks_list_clear().
 *
 * The state should be changed using @ref etrophy_lock_state_set().
 *
 * Example of usage of locks:
 * @code
 * Etrophy_Gamescore *load_function(Eina_List *locks_names)
 * {
 *    Etrophy_Gamescore *gamescore;
 *    gamescore = etrophy_gamescore_load("gamename");
 *    if (!gamescore)
 *      {
 *         const char *name;
 *         Eina_List *l;
 *         gamescore = etrophy_gamescore_new("gamename");
 *         EINA_LIST_FOREACH(locks_names, l, name)
 *           {
 *              Etrophy_Lock *etrophy_lock;
 *              etrophy_lock = etrophy_lock_new(name,
 *                                              ETROPHY_LOCK_STATE_LOCKED);
 *              etrophy_gamescore_lock_add(gamescore, etrophy_lock);
 *           }
 *      }
 *    return gamescore;
 * }
 * void unlock_function(Etrophy_Gamescore *gamescore, const char *name)
 * {
 *    Etrophy_Lock *lock;
 *    lock = etrophy_gamescore_lock_get(gamescore, name);
 *    etrophy_lock_state_set(lock, ETROPHY_LOCK_STATE_UNLOCKED);
 * }
 * @endcode
 *
 */

typedef struct _Etrophy_Lock Etrophy_Lock;
typedef enum
{
   ETROPHY_LOCK_STATE_LOCKED = 0,
   ETROPHY_LOCK_STATE_UNLOCKED,
   ETROPHY_LOCK_STATE_LAST_VALUE
} Etrophy_Lock_State;

EAPI Etrophy_Lock *etrophy_lock_new(const char *name, Etrophy_Lock_State state);
EAPI void etrophy_lock_free(Etrophy_Lock *lock);
EAPI const char *etrophy_lock_name_get(const Etrophy_Lock *lock);
EAPI void etrophy_lock_state_set(Etrophy_Lock *lock, Etrophy_Lock_State state);
EAPI Etrophy_Lock_State etrophy_lock_state_get(const Etrophy_Lock *lock);
EAPI unsigned int etrophy_lock_date_get(const Etrophy_Lock *lock);
EAPI void etrophy_gamescore_lock_add(Etrophy_Gamescore *gamescore, Etrophy_Lock *lock);
EAPI void etrophy_gamescore_lock_del(Etrophy_Gamescore *gamescore, Etrophy_Lock *lock);
EAPI Etrophy_Lock *etrophy_gamescore_lock_get(Etrophy_Gamescore *gamescore, const char *name);
EAPI const Eina_List *etrophy_gamescore_locks_list_get(const Etrophy_Gamescore *gamescore);
EAPI void etrophy_gamescore_locks_list_clear(Etrophy_Gamescore *gamescore);

/**
 * @}
 */

/**
 * @brief Functions that return objects to display gamescore data visually.
 * @defgroup View View
 *
 * @{
 *
 * Beyond data management, Etrophy provides some views, built with Elementary.
 * They consist of Elm Layouts with widgets used to display gamescore data
 * to the application user.
 *
 * It provides views for:
 * @li scores with @ref etrophy_score_layout_add()
 * @li trophies with @ref etrophy_trophies_layout_add()
 * @li locks with @ref etrophy_locks_layout_add()
 *
 */

EAPI Evas_Object *etrophy_score_layout_add(Evas_Object *parent, Etrophy_Gamescore *gamescore);
EAPI Evas_Object *etrophy_trophies_layout_add(Evas_Object *parent, Etrophy_Gamescore *gamescore);
EAPI Evas_Object *etrophy_locks_layout_add(Evas_Object *parent, Etrophy_Gamescore *gamescore);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* __ETROPHY_H__ */
