/************************** Ewings Esp8266 Stack ******************************
This file is part of the Ewings Esp8266 Stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 1st June 2019
******************************************************************************/
#include "EwingsEsp8266Stack.h"

/**
 * initialize all required features/services/actions.
 */
void EwingsEsp8266Stack::initialize(){

  #ifdef EW_SERIAL_LOG
  LogBegin(115200);
  Logln(F("Initializing..."));
  #endif
  __database_service.init_default_database();
  __event_service.begin();
  #if defined( ENABLE_NAPT_FEATURE ) || defined( ENABLE_NAPT_FEATURE_LWIP_V2 )
  __event_service.add_event_listener( EVENT_WIFI_STA_CONNECTED, [&](void*sta_connected) {
    __task_scheduler.setTimeout( [&]() { this->enable_napt_service(); }, NAPT_INIT_DURATION_AFTER_WIFI_CONNECT );
  } );
  #endif

  __wifi_service.begin( this->wifi );
  __ping_service.init_ping( this->wifi );
  #ifdef ENABLE_EWING_HTTP_SERVER
  __web_server.start_server( this->wifi );
  #endif
  __ota_service.begin_ota( &this->wifi_client, &__http_service.client );
  #ifdef ENABLE_GPIO_SERVICE
  __gpio_service.begin( this->wifi, &this->wifi_client );
  #endif
  #ifdef ENABLE_MQTT_SERVICE
  __mqtt_service.begin( this->wifi );
  #endif

  #ifdef ENABLE_EMAIL_SERVICE
  __email_service.begin( this->wifi, &this->wifi_client );
  #endif

  #ifdef EW_SERIAL_LOG
  __task_scheduler.setInterval( this->handleLogPrints, EW_DEFAULT_LOG_DURATION );
  #endif

  __task_scheduler.setInterval( [&]() { __factory_reset.handleFlashKeyPress(); }, FLASH_KEY_PRESS_DURATION );
  __factory_reset.run_while_factory_reset( [&]() { __database_service.clear_default_tables(); this->wifi->disconnect(true); } );

  #ifdef ENABLE_ESP_NOW
  __espnow_service.beginEspNow( this->wifi );
  __task_scheduler.setInterval( [&]() { __espnow_service.handlePeers(); }, ESP_NOW_HANDLE_DURATION );
  #endif

  #ifdef ENABLE_EXCEPTION_NOTIFIER
  beginCrashHandler();
  #endif
}

#if defined( ENABLE_NAPT_FEATURE )
/**
 * enable napt feature
 */
void EwingsEsp8266Stack::enable_napt_service(){

  // Initialize the NAT feature
  ip_napt_init(IP_NAPT_MAX, IP_PORTMAP_MAX);
  // Enable NAT on the AP interface
  ip_napt_enable_no(1, 1);
  // Set the DNS server for clients of the AP to the one we also use for the STA interface
  dhcps_set_DNS(this->wifi->dnsIP());
  #ifdef EW_SERIAL_LOG
    Log(F("NAPT(lwip "));Log(LWIP_VERSION_MAJOR);
    Logln(F(") initialization done"));
  #endif
}
#elif defined( ENABLE_NAPT_FEATURE_LWIP_V2 )
/**
 * enable napt feature
 */
void EwingsEsp8266Stack::enable_napt_service(){

  // Initialize the NAPT feature
  err_t ret = ip_napt_init(IP_NAPT_MAX, IP_PORTMAP_MAX);
  if (ret == ERR_OK) {
    // Enable NAT on the AP interface
    ret = ip_napt_enable_no(SOFTAP_IF, 1);
    if (ret == ERR_OK) {
      #ifdef EW_SERIAL_LOG
        Log(F("NAPT(lwip "));Log(LWIP_VERSION_MAJOR);
        Logln(F(") initialization done"));
      #endif
      // Set the DNS server for clients of the AP to the one we also use for the STA interface
      dhcps_set_dns(0, this->wifi->dnsIP(0));
      dhcps_set_dns(1, this->wifi->dnsIP(1));
    }
  }
  if (ret != ERR_OK) {
    #ifdef EW_SERIAL_LOG
      Logln(F("NAPT initialization failed"));
    #endif
  }
}
#endif

/**
 * serve each internal action, client request, auto operations
 */
void EwingsEsp8266Stack::serve(){

  #ifdef ENABLE_EWING_HTTP_SERVER
  __web_server.handle_clients();
  #endif
  __task_scheduler.handle_tasks();
}

#ifdef EW_SERIAL_LOG
/**
 * prints log as per defined duration
 */
void EwingsEsp8266Stack::handleLogPrints(){

  __wifi_service.printWiFiConfigLogs();
  __ota_service.printOtaConfigLogs();
  #ifdef ENABLE_GPIO_SERVICE
  __gpio_service.printGpioConfigLogs();
  #endif
  #ifdef ENABLE_MQTT_SERVICE
  __mqtt_service.printMqttConfigLogs();
  #endif
  #ifdef ENABLE_EMAIL_SERVICE
  __email_service.printEmailConfigLogs();
  #endif
  __task_scheduler.printTaskSchedulerLogs();
  Log( F("\nNTP Validity : ") );
  Logln( __nw_time_service.is_valid_ntptime() );
  Log( F("NTP Time : ") );
  Logln( __nw_time_service.get_ntp_time() );
}
#endif

EwingsEsp8266Stack EwStack;
