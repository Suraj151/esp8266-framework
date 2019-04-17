#ifndef _SMART_LOO_WORKFLOW_HANDLER_
#define _SMART_LOO_WORKFLOW_HANDLER_

#include <EwingsEsp8266Stack.h>
#include <webserver/pages/SmartLooConfigPage.h>
#include <smartloo/SmartLooConfigs.h>

class SmartLooWorker : public EwingsEsp8266Stack{

	public:

		SmartLooWorker(){
		}

		void begin();
    void run();
		void printSmartLooLogs();

	protected:

    smartloo_config_table smartloo_configs;
		int _smartloo_http_request_cb_id=0;

		void build_smartloo_config_html( char* _page, bool _enable_flash=false, int _max_size=EW_HTML_MAX_SIZE );
    void handleSmartLooConfigRoute( void );

		void setSmartLooHttpRequestRoutine( void );
		void handleSmartLooHttpRequest( void );

		void clear_smarLoo_table_to_defaults( void );
		smartloo_config_table get_smartloo_config_table( void );
		void set_smartloo_config_table( smartloo_config_table* _table );

};

// extern SmartLooWorker SLooWorker;

#endif
