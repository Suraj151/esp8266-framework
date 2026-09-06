/**************************** Time Service ************************************
This file is part of the pdi stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

The one place that decides what time it is. Everything that needs the clock
reads it from here rather than converting an epoch of its own, so two readers
can never disagree about which second they are in.

Where the port can reach an ntp server the time is wall clock and is called
valid. Where it cannot, the millisecond counter still gives a clock that only
moves forward, and that time is simply never called valid — a board with no
network still gets its second and minute ticks.

Author          : Suraj I.
created Date    : 6th Sep 2026
******************************************************************************/
#ifndef _TIME_SERVICE_PROVIDER_H_
#define _TIME_SERVICE_PROVIDER_H_

#include <service_provider/ServiceProvider.h>

class TimeServiceProvider : public ServiceProvider {

public:

  TimeServiceProvider();
  ~TimeServiceProvider();

  bool initService(void *arg = nullptr) override;
  void resetServiceState() override;

  /**
   * Nothing that keeps time can be turned off, because everything that reads
   * the clock would quietly start reading a clock that stopped.
   */
  bool isEssentialService() const override { return true; }

  void printConfigToTerminal(iTerminalInterface *terminal) override;

  /**
   * The instant as of the last tick. Valid says whether the calendar fields
   * are wall clock or only counted from boot.
   */
  const datetime_t &now() const { return m_now; }

  /**
   * Whether the clock is wall clock rather than counted from boot, which is
   * what anything scheduling against a date needs to know.
   */
  bool isTimeValid() const { return m_now.m_valid; }

private:

  /**
   * Reads the clock and announces the boundaries it has crossed, which is the
   * whole of what this service does.
   */
  void tick();

  datetime_t m_now;

  pdiutil::epoch_time_t m_uptime_seconds;
  uint32_t m_last_millis;
  uint32_t m_millis_carry;

  pdiutil::epoch_time_t m_last_second;
  pdiutil::epoch_time_t m_last_minute;
  bool m_announced_valid;
};

extern TimeServiceProvider __time_service;

#endif
