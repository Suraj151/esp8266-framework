/************************ N/W Time Protocol service ***************************
This file is part of the Ewings Esp8266 Stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 1st June 2019
******************************************************************************/

#ifndef _NTP_SERVICE_PROVIDER_H_
#define _NTP_SERVICE_PROVIDER_H_

#include <ESP8266WiFi.h>
#include <config/Config.h>

#define TZ              5       // (utc+) TZ in hours
#define DST_MN          30      // use 60mn for summer time in some countries
#define TZ_MN           ((TZ)*60)
#define TZ_SEC          ((TZ)*3600)
#define DST_SEC         ((DST_MN)*60)

/**
 * NTP servers
 */
#define NTP_SERVER1     "pool.ntp.org"

/**
 * NTPServiceProvider class
 */
class NTPServiceProvider{

  public:

    /**
     * NTPServiceProvider constructor.
     */
    NTPServiceProvider(){

      this->init_ntp_time();
    }

    /**
     * initialize network time
     */
    void init_ntp_time(){

      configTime( TZ_SEC, DST_SEC, NTP_SERVER1 );
    }

    /**
     * check whether dhcp server started and time is valid
     */
    bool is_ntp_in_sync(){

      return (
        wifi_station_dhcpc_status()==DHCP_STARTED && time(nullptr) > LAUNCH_UNIX_TIME
      );
    }

    /**
     * get network time epoch time
     *
     * @return  time_t
     */
    time_t get_ntp_time(){

      return time(nullptr);
    }
};

#endif
