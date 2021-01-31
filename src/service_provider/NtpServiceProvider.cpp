/************************ N/W Time Protocol service ***************************
This file is part of the Ewings Esp Stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 1st June 2019
******************************************************************************/

#include "NtpServiceProvider.h"

/**
 * Set sntp update delay by defining weak function in lwip arduino build
 * default delay of 1hour is used we can change it here and should be > 15 seconds
 */
uint32_t sntp_update_delay_MS_rfc_not_less_than_15000(){
   return 120000; // 120 seconds
}

/**
 * initialize network time
 */
void NTPServiceProvider::init_ntp_time(){

  configTime( TZ_SEC, DST_SEC, NTP_SERVER1 );
  // configTime( TZ_Asia_Kolkata, NTP_SERVER1 );
}

/**
 * check whether ntp time is valid
 */
bool NTPServiceProvider::is_valid_ntptime(){

  return ( time(nullptr) > LAUNCH_UNIX_TIME );
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
