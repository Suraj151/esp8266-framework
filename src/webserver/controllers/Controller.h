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

class Controller;
/**
 * define controller structure here
 */
struct struct_controllers {
	Controller* controller;
	struct_controllers(Controller* _c):controller(_c){}
};

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
			this->register_controller(this);
		}

		/**
		 * Controller constructor
		 */
		Controller(const char* _controller_name){
			this->controller_name = _controller_name;
			this->register_controller(this);
		}

		/**
		 * Controller destructor
		 */
		~Controller(){
		}

		/**
		 * Register Controller
		 */
		void register_controller(Controller* that){
			struct_controllers _c(that);
			this->controllers.push_back(_c);
		}

		/**
		 * override boot
		 */
		virtual void boot() = 0;

		/**
     * @var	std::vector<struct_controllers>	controllers
     */
    static std::vector<struct_controllers> controllers;

		/**
     * @var	const char*	controller_name
     */
    const char*	controller_name = "controller";
};


#endif
