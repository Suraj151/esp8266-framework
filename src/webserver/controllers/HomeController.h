/******************************* Home Controller ******************************
This file is part of the Ewings Esp8266 Stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 1st June 2019
******************************************************************************/

#ifndef _EW_SERVER_HOME_CONTROLLER_
#define _EW_SERVER_HOME_CONTROLLER_

#include <webserver/resources/WebResource.h>
#include <webserver/pages/HomePage.h>

/**
 * HomeController class
 */
class HomeController {

	protected:

		/**
		 * @var	EwWebResourceProvider*	web_resource
		 */
		EwWebResourceProvider* web_resource;

	public:

		/**
		 * HomeController constructor
		 */
		HomeController(){
		}

		/**
		 * HomeController destructor
		 */
		~HomeController(){
		}

		/**
		 * register home, not found route handler
		 *
		 * @param	EwWebResourceProvider*	_web_resource
		 */
		void handle( EwWebResourceProvider* _web_resource ){

			this->web_resource = _web_resource;
			this->web_resource->register_route( EW_SERVER_HOME_ROUTE, [&]() { this->handleHomeRoute(); } );
      this->web_resource->register_not_found_fn( [&]() { this->handleNotFound(); } );
		}

		/**
		 * build html page with header, middle and footer part.
		 *
		 * @param	char*	_page
		 * @param	PGM_P	_pgm_page
		 * @param	bool|false	_enable_flash
		 * @param	char*|""	_message
		 * @param	FLASH_MSG_TYPE|ALERT_SUCCESS	_alert_type
		 * @param	bool|true	_enable_header_footer
		 * @param	int|EW_HTML_MAX_SIZE	_max_size
		 */
		void build_html(
      char* _page,
      PGM_P _pgm_page,
      bool _enable_flash=false,
      char* _message="",
      FLASH_MSG_TYPE _alert_type=ALERT_SUCCESS ,
      bool _enable_header_footer=true,
      int _max_size=EW_HTML_MAX_SIZE
    ){

      memset( _page, 0, _max_size );

      if( _enable_header_footer ) strcat_P( _page, EW_SERVER_HEADER_HTML );
      strcat_P( _page, _pgm_page );
      if( _enable_flash )
      concat_flash_message_div( _page, _message, _alert_type );
      if( _enable_header_footer ) strcat_P( _page, EW_SERVER_FOOTER_HTML );
      // Log(F("free stack :"));Logln(ESP.getFreeContStack());
      // Log(F("free heap :"));Logln(ESP.getFreeHeap());
    }

		/**
		 * build and send not found page to client
		 */
    void handleNotFound( void ) {
      #ifdef EW_SERIAL_LOG
      Logln(F("Handling 404 route"));
      #endif

      char* _page = new char[EW_HTML_MAX_SIZE];
      this->build_html( _page, EW_SERVER_404_PAGE );

      this->web_resource->EwServer->send( HTTP_NOT_FOUND, EW_HTML_CONTENT, _page );
      delete[] _page;
    }

		/**
		 * build and send home page to client
		 */
    void handleHomeRoute( void ) {

      #ifdef EW_SERIAL_LOG
      Logln(F("Handling Home route"));
      #endif

      char* _page = new char[EW_HTML_MAX_SIZE];
      this->build_html( _page, this->web_resource->has_active_session() ? EW_SERVER_HOME_AUTHORIZED_PAGE : EW_SERVER_HOME_PAGE );

      this->web_resource->EwServer->send( HTTP_OK, EW_HTML_CONTENT, _page );
      delete[] _page;
    }

};

#endif
