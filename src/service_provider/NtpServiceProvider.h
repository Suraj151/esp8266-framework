#ifndef _NTP_SERVICE_PROVIDER_H_
#define _NTP_SERVICE_PROVIDER_H_

#include <ESP8266WiFi.h>
#include <database/StoreStruct.h>

#define TZ              5       // (utc+) TZ in hours
#define DST_MN          30      // use 60mn for summer time in some countries


#define TZ_MN           ((TZ)*60)
#define TZ_SEC          ((TZ)*3600)
#define DST_SEC         ((DST_MN)*60)

#define NTP_SERVER1     "pool.ntp.org"

class NTPServiceProvider{

  public:

    NTPServiceProvider(){

      this->init_ntp_time();
    }

    void init_ntp_time(){

      configTime( TZ_SEC, DST_SEC, NTP_SERVER1 );
    }

    bool is_ntp_in_sync(){

      return (
        wifi_station_dhcpc_status()==DHCP_STARTED && time(nullptr) > LAUNCH_UNIX_TIME
      );
    }

    time_t get_ntp_time(){

      return time(nullptr);
    }
};

#endif
