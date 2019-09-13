/*************************** Dashboard Controller *****************************
This file is part of the Ewings Esp8266 Stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 1st June 2019
******************************************************************************/

#ifndef _EW_SERVER_DASHBOARD_CONTROLLER_
#define _EW_SERVER_DASHBOARD_CONTROLLER_

#include <webserver/resources/WebResource.h>
#include <webserver/pages/Dashboard.h>

/**
 * DashboardController class
 */
class DashboardController {

	protected:

		/**
		 * @var	EwWebResourceProvider*	web_resource
		 */
		EwWebResourceProvider* web_resource;

	public:

		/**
		 * DashboardController constructor
		 */
		DashboardController(){
		}

		/**
		 * DashboardController destructor
		 */
		~DashboardController(){
		}

		/**
		 * register dashboard route handler
		 *
		 * @param	EwWebResourceProvider*	_web_resource
		 */
		void handle( EwWebResourceProvider* _web_resource ){

			this->web_resource = _web_resource;
			this->web_resource->register_route( EW_SERVER_DASHBOARD_ROUTE, [&]() { this->handleDashboardRoute(); }, AUTH_MIDDLEWARE );
      this->web_resource->register_route( EW_SERVER_DASHBOARD_MONITOR_ROUTE, [&]() { this->handleDashboardMonitor(); } );
		}

		/**
		 * handle dashboard monitor calls. it provides live dashborad parameters.
		 */
		void handleDashboardMonitor( void ) {

      #ifdef EW_SERIAL_LOG
      Logln(F("Handling dashboard monitor route"));
      #endif

      String _response = "{\"nm\":\"";
      _response += this->web_resource->wifi->SSID();
			_response += "\",\"ip\":\"";
      _response += this->web_resource->wifi->localIP().toString();
      _response += "\",\"rs\":\"";
      _response += this->web_resource->wifi->RSSI();
      _response += "\",\"mc\":\"";
      _response += this->web_resource->wifi->macAddress();
      _response += "\",\"st\":";
      _response += this->web_resource->wifi->status();
      _response += ",\"r\":";
      _response += !this->web_resource->has_active_session();
      _response += "}";

      this->web_resource->EwServer->sendHeader("Cache-Control", "no-cache");
      this->web_resource->EwServer->send( HTTP_OK, EW_HTML_CONTENT, _response );
    }

		/**
		 * handle dashboard page route. it build & send dashboard html to client.
		 */
    void handleDashboardRoute( void ) {
      #ifdef EW_SERIAL_LOG
      Logln(F("Handling dashboard route"));
      #endif
      bool _is_posted = false;

      char* _page = new char[EW_HTML_MAX_SIZE];
      memset( _page, 0, EW_HTML_MAX_SIZE );

      strcat_P( _page, EW_SERVER_HEADER_HTML );
      strcat_P( _page, EW_SERVER_DASHBOARD_PAGE );
      strcat_P( _page, EW_SERVER_FOOTER_WITH_DASHBOARD_MONITOR_HTML );

      this->web_resource->EwServer->send( HTTP_OK, EW_HTML_CONTENT, _page );
      delete[] _page;

    }

};

#endif
