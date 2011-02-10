/*
 * Copyright 2011 Mike Blumenkrantz <mike@zentific.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <Esskyuehl.h>
#include <esql_private.h>

/**
 * @defgroup Esql_Library Library
 * @brief Functions to control initialization of the library
 * @{*/
int esql_log_dom = -1;
static int esql_init_count_ = 0;

int ESQL_EVENT_ERROR = 0;
int ESQL_EVENT_CONNECT = 0;
int ESQL_EVENT_RESULT = 0;


/**
 * @brief Initialize Esql
 * 
 * This function initializes events and
 * logging functions for Esql and must be called prior to making any
 * calls.
 * @return The number of times the function has been called, or -1 on failure
 */
int
esql_init(void)
{
   if (++esql_init_count_ != 1)
     return esql_init_count_;
   if (!eina_init()) return 0;
   esql_log_dom = eina_log_domain_register("esskyuehl", EINA_COLOR_BLUE);
   if (esql_log_dom < 0)
     {
        ERR("Could not register 'esql' log domain!");
        goto eina_fail;
     }
   if (!ecore_init()) goto fail;

   ESQL_EVENT_ERROR = ecore_event_type_new();
   ESQL_EVENT_RESULT = ecore_event_type_new();
   ESQL_EVENT_CONNECT = ecore_event_type_new();

   return esql_init_count_;

fail:
   eina_log_domain_unregister(esql_log_dom);
   esql_log_dom = -1;
eina_fail:
   eina_shutdown();
   return 0;
}

/**
 * @brief Shut down Esql
 * 
 * This function uninitializes memory allocated by esql_init.
 * Call when no further Esql functions will be used.
 * @return The number of times the esql_init has been called, or -1 if 
 * all occurrences of esql have been shut down
 */
int
esql_shutdown(void)
{
   if (--esql_init_count_ != 0)
     return esql_init_count_;

   eina_log_domain_unregister(esql_log_dom);
   ecore_shutdown();
   eina_shutdown();
   esql_log_dom = -1;
   return esql_init_count_;
}
/** @} */
/**
 * @defgroup Esql_Object Objects
 * @brief Functions to control Esql objects
 * @{*/

/**
 * @brief Create a new #Esql object
 * @param The database type to use
 * This function does nothing but allocate a generic Esskyuehl struct.
 * @return The new object, or #NULL on failure
 */
Esql *
esql_new(Esql_Type type)
{
   Esql *e;

   e = calloc(1, sizeof(Esql));
   EINA_SAFETY_ON_NULL_RETURN_VAL(e, NULL);
   esql_type_set(e, type);

   return e;
}

/**
 * @brief Retrieve data previously associated with an object
 * @param e The #Esql object (NOT #NULL)
 * @return The data
 */
void *
esql_data_get(Esql *e)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(e, NULL);
   return e->data;
}

/**
 * @brief Associate data with an #Esql object for later use
 * @param e The #Esql object (NOT #NULL)
 * @param data The data to associate
 */
void
esql_data_set(Esql *e, void *data)
{
   EINA_SAFETY_ON_NULL_RETURN(e);
   e->data = data;
}

/**
 * @brief Set the database type of an #Esql object
 * This function sets up all necessary function hooks to use @p e with a database
 * type specified by @p type.
 * @note If this function is called on an object multiple times, any connections associated
 * with the object will be immediately lost.
 * @param e The #Esql object (NOT #NULL)
 * @param type The type of database to use.
 * @return EINA_TRUE on success, else EINA_FALSE
 */
Eina_Bool
esql_type_set(Esql *e, Esql_Type type)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(e, EINA_FALSE);

   if ((type != e->type) && e->backend.db && e->backend.free)
     e->backend.free(e);

   memset(&e->backend, 0, sizeof(e->backend));
   switch (type)
     {
      case ESQL_TYPE_MYSQL:
        esql_mysac_init(e);
        break;
      case ESQL_TYPE_POSTGRESQL:
        esql_postgresql_init(e);
        break;
      default:
        return EINA_FALSE;
     }
   return EINA_TRUE;
}

/**
 * @brief Return the database type of an #Esql object
 * @see esql_type_set
 * @param e The #Esql object (NOT #NULL)
 * @return The database type currently used by @p e
 */
Esql_Type
esql_type_get(Esql *e)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(e, ESQL_TYPE_NONE);

   return e->type;
}

/**
 * @brief Free an #Esql object
 * This function frees an #Esql object, shutting down all
 * open connections and freeing all db-related data.
 * @param e The #Esql object (NOT #NULL)
 */
void
esql_free(Esql *e)
{
   EINA_SAFETY_ON_NULL_RETURN(e);
   if (e->connected) esql_disconnect(e);
   if (e->backend.free) e->backend.free(e);
   free(e);
}
 /** @} */
