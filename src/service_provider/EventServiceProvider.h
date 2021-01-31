/****************************** Event service *********************************
This file is part of the Ewings Esp Stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 1st June 2019
******************************************************************************/

#ifndef _EVENT_SERVICE_PROVIDER_H_
#define _EVENT_SERVICE_PROVIDER_H_


#include <service_provider/ServiceProvider.h>

/**
 * EventServiceProvider class
 */
class EventServiceProvider : public ServiceProvider {

  public:
    /**
     * EventServiceProvider constructor.
     */
    EventServiceProvider();
    /**
		 * EventServiceProvider destructor
		 */
    ~EventServiceProvider();

    void begin(void);
    bool add_event_listener( event_name_t _event, CallBackVoidPointerArgFn _handler );
    void execute_event( event_name_t _event, void* _arg=nullptr );
    static void wifi_event_handler_cb( System_Event_t * _event );

    int           m_last_event;
    unsigned long m_last_event_millis;

  protected:

		/**
		 * @var	std::vector<event_listener_t> vector
		 */
		std::vector<event_listener_t> m_event_listeners;
};

extern EventServiceProvider __event_service;

#endif
