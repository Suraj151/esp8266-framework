/************************ N/W Time Protocol service ***************************
This file is part of the Ewings Esp Stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 1st June 2019
******************************************************************************/

#ifndef _NTP_SERVICE_PROVIDER_H_
#define _NTP_SERVICE_PROVIDER_H_


// #include <TZ.h>
#include <service_provider/ServiceProvider.h>

#ifndef TZ_Asia_Kolkata
#define TZ_Asia_Kolkata	PSTR("IST-5:30")
#endif

#define TZ              5.5       // (utc+) TZ in hours
#define TZ_MN           ((TZ)*60)
#define TZ_SEC          ((TZ)*3600)
#define DST_MN          0      // use 60mn for summer time in some countries
#define DST_SEC         ((DST_MN)*60)

/**
 * NTP servers
 */
#define NTP_SERVER1     "pool.ntp.org"

/**
 * NTPServiceProvider class
 */
class NTPServiceProvider : public ServiceProvider {

  public:

    /**
     * NTPServiceProvider constructor.
     */
    NTPServiceProvider(){

      this->init_ntp_time();
    }

    /**
		 * NTPServiceProvider destructor
		 */
    ~NTPServiceProvider(){
    }

    void init_ntp_time( void );
    bool is_valid_ntptime( void );
    time_t get_ntp_time( void );
};

extern NTPServiceProvider __nw_time_service;

#endif
