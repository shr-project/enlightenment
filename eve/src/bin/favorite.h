/* This file has been automatically generated by geneet.py */
/*                      DO NOT MODIFY                      */

#ifndef __FAVORITE_H__
#define __FAVORITE_H__

#include <Eina.h>
#include <Eet.h>

typedef struct _Fav_Item Fav_Item;
typedef struct _Fav Fav;

/* Fav_Item */
Fav_Item *fav_item_new(const char * url, const char * title, unsigned int visit_count);
void fav_item_free(Fav_Item *fav_item);

void fav_item_url_set(Fav_Item *fav_item, const char * url);
const char * fav_item_url_get(const Fav_Item *fav_item);
void fav_item_title_set(Fav_Item *fav_item, const char * title);
const char * fav_item_title_get(const Fav_Item *fav_item);
void fav_item_visit_count_set(Fav_Item *fav_item, unsigned int visit_count);
unsigned int fav_item_visit_count_get(const Fav_Item *fav_item);

/* Fav */
Fav *fav_new(int version);
void fav_free(Fav *fav);

void fav_version_set(Fav *fav, int version);
int fav_version_get(const Fav *fav);
void fav_items_add(Fav *fav, const char * url, Fav_Item *fav_item);
void fav_items_del(Fav *fav, const char * url);
Fav_Item *fav_items_get(const Fav *fav, const char * key);
Eina_Hash *fav_items_hash_get(const Fav *fav);
void fav_items_modify(Fav *fav, const char * key, void *value);

Fav *fav_load(const char *filename);
Eina_Bool fav_save(Fav *fav, const char *filename);

/* Global initializer / shutdown functions */
void favorite_init(void);
void favorite_shutdown(void);

#endif /* __FAVORITE_H__ */
