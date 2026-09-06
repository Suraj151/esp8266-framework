/**************************** Time Service ************************************
This file is part of the pdi stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 6th Sep 2026
******************************************************************************/
#include "TimeServiceProvider.h"
#include <utility/DataTypeConversions.h>
#include <utility/EventUtil.h>

#ifdef DEVICE_SUPPORTS_NTP
#include <interface/pdi/middlewares/iNtpInterface.h>
#endif

/**
 * TimeServiceProvider constructor
 */
TimeServiceProvider::TimeServiceProvider() : ServiceProvider(SERVICE_TIME, RODT_ATTR("Time")),
  m_uptime_seconds(0), m_last_millis(0), m_millis_carry(0),
  m_last_second(-1), m_last_minute(-1), m_announced_valid(false)
{
}

/**
 * TimeServiceProvider destructor
 */
TimeServiceProvider::~TimeServiceProvider()
{
}

/**
 * initialize the service
 */
bool TimeServiceProvider::initService(void *arg)
{
  m_last_millis = __i_dvc_ctrl.millis_now();

  this->serviceSetInterval([&](){
    this->tick();
  }, TIME_SERVICE_TICK_MS, __i_dvc_ctrl.millis_now(), MEDIUM_TASK_PRIORITY);

  return ServiceProvider::initService(arg);
}

/**
 * return the service to the state it started from, so a restart begins cold
 */
void TimeServiceProvider::resetServiceState()
{
  m_now = datetime_t();
  m_uptime_seconds = 0;
  m_last_millis = __i_dvc_ctrl.millis_now();
  m_millis_carry = 0;
  m_last_second = -1;
  m_last_minute = -1;
  m_announced_valid = false;
}

/**
 * Reads the clock and announces the boundaries it has crossed, which is the
 * whole of what this service does.
 */
void TimeServiceProvider::tick()
{
  uint32_t nowmillis = __i_dvc_ctrl.millis_now();
  uint32_t elapsed = nowmillis - m_last_millis;
  m_last_millis = nowmillis;

  m_millis_carry += elapsed;
  if( m_millis_carry >= MILLISECOND_DURATION_1000 ){
    m_uptime_seconds += (pdiutil::epoch_time_t)(m_millis_carry / MILLISECOND_DURATION_1000);
    m_millis_carry = m_millis_carry % MILLISECOND_DURATION_1000;
  }

  bool valid = false;
  pdiutil::epoch_time_t utc = m_uptime_seconds;

#ifdef DEVICE_SUPPORTS_NTP
  if( __i_ntp.is_valid_ntptime() ){
    valid = true;
    utc = __i_ntp.get_ntp_time();
  }
#endif

  EpochToDateTime((uint32_t)(utc + (pdiutil::epoch_time_t)TZ_SEC), m_now);
  m_now.m_epoch = utc;
  m_now.m_valid = valid;

  if( valid != m_announced_valid ){
    m_announced_valid = valid;

    m_last_second = -1;
    m_last_minute = -1;

    __utl_event.execute_event(EVENT_TIME_SYNC, (void*)&m_now);
  }

  pdiutil::epoch_time_t second = utc;
  pdiutil::epoch_time_t minute = utc / 60;

  if( second != m_last_second ){
    m_last_second = second;
    __utl_event.execute_event(EVENT_TIME_SECOND, (void*)&m_now);
  }

  if( minute != m_last_minute ){
    m_last_minute = minute;
    __utl_event.execute_event(EVENT_TIME_MINUTE, (void*)&m_now);
  }
}

/**
 * print the service configuration to the terminal
 */
void TimeServiceProvider::printConfigToTerminal(iTerminalInterface *terminal)
{
  if( nullptr == terminal ){
    return;
  }

  char stamp[24];
  EpochToDateTimeString((uint32_t)(m_now.m_epoch + (pdiutil::epoch_time_t)TZ_SEC), stamp, sizeof(stamp));

  terminal->write_ro(RODT_ATTR("\nsource: "));
#ifdef DEVICE_SUPPORTS_NTP
  terminal->write_ro(m_now.m_valid ? RODT_ATTR("ntp") : RODT_ATTR("uptime, ntp not synced"));
#else
  terminal->write_ro(RODT_ATTR("uptime, this port has no ntp"));
#endif

  terminal->write_ro(RODT_ATTR("\nlocal : "));
  terminal->writeln(m_now.m_valid ? stamp : NOT_APPLICABLE);

  terminal->write_ro(RODT_ATTR("uptime: "));
  terminal->write((uint32_t)m_uptime_seconds);
  terminal->writeln_ro(RODT_ATTR("s"));
}

/**
 * Global instance of the time service.
 */
TimeServiceProvider __time_service;
