/************************ N/W Time Protocol service ***************************
This file is part of the Ewings Esp8266 Stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 1st June 2019
******************************************************************************/

#include "NtpServiceProvider.h"

/**
 * initialize network time
 */
void NTPServiceProvider::init_ntp_time(){

  configTime( TZ_SEC, DST_SEC, NTP_SERVER1 );
  // configTime( TZ_Asia_Kolkata, NTP_SERVER1 );
}

/**
 * check whether dhcp server started and time is valid
 */
bool NTPServiceProvider::is_ntp_in_sync(){

  return (
    wifi_station_dhcpc_status()==DHCP_STARTED && time(nullptr) > LAUNCH_UNIX_TIME
  );
}

/**
 * get network time epoch time
 *
 * @return  time_t
 */
time_t NTPServiceProvider::get_ntp_time(){

  return time(nullptr);
}

NTPServiceProvider __nw_time_service;
