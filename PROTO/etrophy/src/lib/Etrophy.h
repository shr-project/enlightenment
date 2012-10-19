#ifndef __ETROPHY_H__
#define __ETROPHY_H__

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

typedef struct _Etrophy_Trophy Etrophy_Trophy;
typedef enum
{
   ETROPHY_TROPHY_STATE_HIDDEN = 0,
   ETROPHY_TROPHY_STATE_VISIBLE,
   ETROPHY_TROPHY_STATE_LAST_VALUE
} Etrophy_Trophy_Visibility;

typedef struct _Etrophy_Lock Etrophy_Lock;
typedef enum
{
   ETROPHY_LOCK_STATE_LOCKED = 0,
   ETROPHY_LOCK_STATE_UNLOCKED,
   ETROPHY_LOCK_STATE_LAST_VALUE
} Etrophy_Lock_State;

typedef struct _Etrophy_Score     Etrophy_Score;
typedef struct _Etrophy_Level     Etrophy_Level;
typedef struct _Etrophy_Gamescore Etrophy_Gamescore;

/* Etrophy_Trophy */
EAPI Etrophy_Trophy           *etrophy_trophy_new(const char *name, const char *description, Etrophy_Trophy_Visibility visibility, unsigned int goal);
EAPI void                      etrophy_trophy_free(Etrophy_Trophy *trophy);

EAPI const char               *etrophy_trophy_name_get(const Etrophy_Trophy *trophy);
EAPI const char               *etrophy_trophy_description_get(const Etrophy_Trophy *trophy);
EAPI Etrophy_Trophy_Visibility etrophy_trophy_visibility_get(const Etrophy_Trophy *trophy);
EAPI void                      etrophy_trophy_goal_get(const Etrophy_Trophy *trophy, unsigned int *goal, unsigned int *counter);
EAPI void                      etrophy_trophy_counter_set(Etrophy_Trophy *trophy, unsigned int value);
EAPI void                      etrophy_trophy_counter_increment(Etrophy_Trophy *trophy, unsigned int value);
EAPI Eina_Bool                 etrophy_trophy_earned_get(const Etrophy_Trophy *trophy);
EAPI unsigned int              etrophy_trophy_date_get(const Etrophy_Trophy *trophy);

/* Etrophy_Lock */
EAPI Etrophy_Lock             *etrophy_lock_new(const char *name, Etrophy_Lock_State state);
EAPI void                      etrophy_lock_free(Etrophy_Lock *lock);

EAPI const char               *etrophy_lock_name_get(const Etrophy_Lock *lock);
EAPI void                      etrophy_lock_state_set(Etrophy_Lock *lock, Etrophy_Lock_State state);
EAPI Etrophy_Lock_State        etrophy_lock_state_get(const Etrophy_Lock *lock);
EAPI unsigned int              etrophy_lock_date_get(const Etrophy_Lock *lock);

/* Etrophy_Score */
EAPI Etrophy_Score            *etrophy_score_new(const char *player_name, int score);
EAPI void                      etrophy_score_free(Etrophy_Score *escore);

EAPI const char               *etrophy_score_player_name_get(const Etrophy_Score *escore);
EAPI int                       etrophy_score_score_get(const Etrophy_Score *escore);
EAPI unsigned int              etrophy_score_date_get(const Etrophy_Score *escore);

/* Etrophy_Level */
EAPI Etrophy_Level            *etrophy_level_new(const char *name);
EAPI void                      etrophy_level_free(Etrophy_Level *level);

EAPI const char               *etrophy_level_name_get(const Etrophy_Level *level);
EAPI void                      etrophy_level_score_add(Etrophy_Level *level, Etrophy_Score *escore);
EAPI void                      etrophy_level_score_del(Etrophy_Level *level, Etrophy_Score *escore);
EAPI const Eina_List          *etrophy_level_scores_list_get(const Etrophy_Level *level);
EAPI void                      etrophy_level_scores_list_clear(Etrophy_Level *level);

/* Etrophy_Gamescore */
EAPI Etrophy_Gamescore        *etrophy_gamescore_new(const char *gamename);
EAPI void                      etrophy_gamescore_free(Etrophy_Gamescore *gamescore);
EAPI Eet_Data_Descriptor      *etrophy_gamescore_edd_get(void);
EAPI Etrophy_Gamescore        *etrophy_gamescore_path_load(const char *filename);
EAPI Etrophy_Gamescore        *etrophy_gamescore_load(const char *gamename);
EAPI Eina_Bool                 etrophy_gamescore_save(Etrophy_Gamescore *gamescore, const char *filename);

EAPI void                      etrophy_gamescore_level_add(Etrophy_Gamescore *gamescore, Etrophy_Level *level);
EAPI void                      etrophy_gamescore_level_del(Etrophy_Gamescore *gamescore, Etrophy_Level *level);
EAPI Etrophy_Level            *etrophy_gamescore_level_get(Etrophy_Gamescore *gamescore, const char *name);
EAPI const Eina_List          *etrophy_gamescore_levels_list_get(const Etrophy_Gamescore *gamescore);
EAPI void                      etrophy_gamescore_levels_list_clear(Etrophy_Gamescore *gamescore);

EAPI void                      etrophy_gamescore_trophy_add(Etrophy_Gamescore *gamescore, Etrophy_Trophy *trophy);
EAPI void                      etrophy_gamescore_trophy_del(Etrophy_Gamescore *gamescore, Etrophy_Trophy *trophy);
EAPI Etrophy_Trophy           *etrophy_gamescore_trophy_get(Etrophy_Gamescore *gamescore, const char *name);
EAPI const Eina_List          *etrophy_gamescore_trophies_list_get(const Etrophy_Gamescore *gamescore);
EAPI void                      etrophy_gamescore_trophies_list_clear(Etrophy_Gamescore *gamescore);

EAPI void                      etrophy_gamescore_lock_add(Etrophy_Gamescore *gamescore, Etrophy_Lock *lock);
EAPI void                      etrophy_gamescore_lock_del(Etrophy_Gamescore *gamescore, Etrophy_Lock *lock);
EAPI Etrophy_Lock             *etrophy_gamescore_lock_get(Etrophy_Gamescore *gamescore, const char *name);
EAPI const Eina_List          *etrophy_gamescore_locks_list_get(const Etrophy_Gamescore *gamescore);
EAPI void                      etrophy_gamescore_locks_list_clear(Etrophy_Gamescore *gamescore);

EAPI void                      etrophy_gamescore_clear(Etrophy_Gamescore *gamescore);

EAPI int                       etrophy_gamescore_level_hi_score_get(const Etrophy_Gamescore *gamescore, const char *level_name);
EAPI int                       etrophy_gamescore_level_low_score_get(const Etrophy_Gamescore *gamescore, const char *level_name);
EAPI Etrophy_Score            *etrophy_gamescore_level_score_add(Etrophy_Gamescore *gamescore, const char *level_name, const char *player_name, int score);

/* Layouts */
EAPI Evas_Object              *etrophy_score_layout_add(Evas_Object *parent, Etrophy_Gamescore *gamescore);
EAPI Evas_Object              *etrophy_trophies_layout_add(Evas_Object *parent, Etrophy_Gamescore *gamescore);
EAPI Evas_Object              *etrophy_locks_layout_add(Evas_Object *parent, Etrophy_Gamescore *gamescore);

/* Global initializer / shutdown functions */
EAPI int                       etrophy_init(void);
EAPI int                       etrophy_shutdown(void);

#ifdef __cplusplus
}
#endif

#endif /* __ETROPHY_H__ */
