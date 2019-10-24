/****************************** Event service *********************************
This file is part of the Ewings Esp8266 Stack.

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

    int _last_event;
    unsigned long _last_event_millis;

    /**
     * EventServiceProvider constructor.
     */
    EventServiceProvider(){
    }

    /**
		 * EventServiceProvider destructor
		 */
    ~EventServiceProvider(){
    }

    void begin(void);
    bool add_event_listener( event_name_t _event, CallBackVoidPointerArgFn _handler );
    void execute_event( event_name_t _event, void* _arg=NULL );
    static void wifi_event_handler_cb( System_Event_t * _event );

  protected:

		/**
		 * @var	std::vector<event_listener_t> vector
		 */
		std::vector<event_listener_t> _event_listeners;
};

extern EventServiceProvider __event_service;

#endif
