/*********************************** Controller *******************************
This file is part of the Ewings Esp8266 Stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 1st June 2019
******************************************************************************/

#ifndef _SERVER_CONTROLLER_
#define _SERVER_CONTROLLER_

#include <webserver/handlers/RouteHandler.h>

/**
 * Controller class
 */
class Controller {

	protected:

		/**
		 * @var	WebResourceProvider*	web_resource
		 */
		WebResourceProvider* web_resource = &__web_resource;
		/**
		 * @var	RouteHandler*	route_handler
		 */
		RouteHandler* route_handler = &__web_route_handler;

	public:

		/**
		 * Controller constructor
		 */
		Controller(){
		}

		/**
		 * Controller destructor
		 */
		~Controller(){
		}
};

#endif
