#ifndef ECOREXX_EVENT_H
#define ECOREXX_EVENT_H

/* EFL */
#include <Ecore.h>

/* SIGC */
#include <sigc++/sigc++.h>

namespace Ecorexx {

class Event
{
public:
  Event(int type, sigc::slot<bool, int, void*> handler);
  virtual ~Event();
  
  /**
   * @brief Add an event to the event queue.
   * @param type The event type to add to the end of the event queue
   * @param ev The data structure passed as @c event to event handlers
   * @param func_free The function to be called to free @a ev
   * @param data The data pointer to be passed to the free function
   * @return A Handle for that event on success, otherwise NULL
   *
   * If it succeeds, an event of type @a type will be added to the queue for
   * processing by event handlers added by ecore_event_handler_add(). The @a ev
   * parameter will be passed as the @c event parameter of the handler. When the
   * event is no longer needed, @a func_free will be called and passed @a ev for
   * cleaning up. If @p func_free is NULL, free() will be called with the private
   * structure pointer.
   */
  //EAPI Ecore_Event *ecore_event_add(int type, void *ev, Ecore_End_Cb func_free, void *data);
  
  /**
   * @brief Delete an event from the queue.
   * @param event The event handle to delete
   * @return The data pointer originally set for the event free function
   *
   * This deletes the event @p event from the event queue, and returns the
   * @p data parameter originally set when adding it with ecore_event_add(). This
   * does not immediately call the free function, and it may be called later on
   * cleanup, and so if the free function depends on the data pointer to work,
   * you should defer cleaning of this till the free function is called later.
   */
  //EAPI void *ecore_event_del(Ecore_Event *event);
  
  /**
   * @brief Get the data associated with an #Ecore_Event_Handler
   * @param eh The event handler
   * @return The data
   *
   * This function returns the data previously associated with @p eh by
   * ecore_event_handler_add().
   */
  //EAPI void *ecore_event_handler_data_get(Ecore_Event_Handler *eh);
  
  /**
   * @brief Set the data associated with an #Ecore_Event_Handler
   * @param eh The event handler
   * @param data The data to associate
   * @return The previous data
   *
   * This function sets @p data to @p eh and returns the old data pointer
   * which was previously associated with @p eh by ecore_event_handler_add().
   */
  //EAPI void *ecore_event_handler_data_set(Ecore_Event_Handler *eh, const void *data);
  
  /**
   * @brief Allocate a new event type id sensibly and return the new id.
   * @return A new event type id.
   *
   * This function allocates a new event type id and returns it. Once an event
   * type has been allocated it can never be de-allocated during the life of
   * the program. There is no guarantee of the contents of this event ID, or how
   * it is calculated, except that the ID will be unique to the current instance
   * of the process.
   */
  //EAPI int ecore_event_type_new(void);
  
  /**
   * @brief Add a filter the current event queue.
   *
   * @param func_start Function to call just before filtering and return data
   * @param func_filter Function to call on each event
   * @param func_end Function to call after the queue has been filtered
   * @param data Data to pass to the filter functions
   * @return A filter handle on success, @c NULL otherwise.
   *
   * Adds a callback to filter events from the event queue. Filters are called on
   * the queue just before Event handler processing to try and remove redundant
   * events. Just as processing is about to start @a func_start is called and
   * passed the @a data pointer, the return value of this functions is passed to
   * @a func_filter as loop_data. @a func_filter is also passed @a data and the
   * event type and event structure. If this @a func_filter returns
   * @c EINA_FALSE, the event is removed from the queue, if it returns
   * @c EINA_TRUE, the event is kept. When processing is finished @p func_end is
   * called and is passed the loop_data(returned by @c func_start) and @p data
   * pointer to clean up.
   */
  //EAPI Ecore_Event_Filter *ecore_event_filter_add(Ecore_Data_Cb func_start, Ecore_Filter_Cb func_filter, Ecore_End_Cb func_end, const void *data);
  
  /**
   * @brief Delete an event filter.
   * @param ef The event filter handle
   * @return The data set for the filter on success, @c NULL otherwise.
   *
   * Delete a filter that has been added by its @p ef handle.
   */
  //EAPI void *ecore_event_filter_del(Ecore_Event_Filter *ef);
  
  /**
   * @brief Return the current event type being handled.
   * @return The current event type being handled if inside a handler callback,
   * ECORE_EVENT_NONE otherwise
   *
   * If the program is currently inside an Ecore event handler callback this
   * will return the type of the current event being processed.
   *
   * This is useful when certain Ecore modules such as Ecore_Evas "swallow"
   * events and not all the original information is passed on. In special cases
   * this extra information may be useful or needed and using this call can let
   * the program know if the event type being handled is one it wants to get more
   * information about.
   */
  //EAPI int ecore_event_current_type_get(void);
  
  /**
   * @brief Return the current event type pointer handled.
   * @return The current event pointer being handled if inside a handler callback,
   * @c NULL otherwise.
   *
   * If the program is currently inside an Ecore event handler callback this
   * will return the pointer of the current event being processed.
   *
   * This is useful when certain Ecore modules such as Ecore_Evas "swallow"
   * events and not all the original information is passed on. In special cases
   * this extra information may be useful or needed and using this call can let
   * the program access the event data if the type of the event is handled by
   * the program.
   */
  //EAPI void *ecore_event_current_event_get(void);

  sigc::slot<bool, int, void*> mHandler;
  
private:
  static Eina_Bool eventHandler(void *data, int type, void *event);
  
  Ecore_Event_Handler *mEventHandler;
  
};

} // end namespace Ecorexx

#endif // ECOREXX_EVENT_H
