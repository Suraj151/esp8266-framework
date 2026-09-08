/**************************** Cron Service ************************************
This file is part of the pdi stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Runs the jobs in /etc/crontab on the minute they name. The table is read from
the file every minute and nothing is held between runs, so an edit takes effect
on the next minute and a board with a small heap carries no cost for a feature
it is not using.

Jobs only run while the clock is trustworthy. A board with no time source keeps
counting forward but never calls itself valid, and a calendar cannot be matched
against a count from boot.

Author          : Suraj I.
created Date    : 7th Sep 2026
******************************************************************************/
#ifndef _CRON_SERVICE_PROVIDER_H_
#define _CRON_SERVICE_PROVIDER_H_

#include <service_provider/ServiceProvider.h>

#ifdef ENABLE_CRON_SERVICE

class CronServiceProvider : public ServiceProvider {

public:

  CronServiceProvider();
  ~CronServiceProvider();

  bool initService(void *arg = nullptr) override;
  bool stopService() override;
  void resetServiceState() override;

  void printConfigToTerminal(iTerminalInterface *terminal) override;

  /**
   * Walks the table once against the instant given, running whatever matches.
   * Public so the shell can ask what would run without waiting for the minute.
   */
  uint8_t runDueJobs(const datetime_t &now, bool dryrun = false,
                     iTerminalInterface *terminal = nullptr);

  /**
   * Reads one table row into the five field specs and the command, false when
   * the line is blank, a comment, or does not carry all of them.
   */
  static bool parseRow(const pdiutil::string &line, pdiutil::string *fields,
                       pdiutil::string &command);

  /**
   * Reads a decimal out of a field item, false unless every character of it is
   * a digit, so a spec that is not a number cannot read as zero.
   */
  static bool parseNumber(const pdiutil::string &text, uint32_t &out);

  /**
   * Whether a field spec covers the value, over the standard grammar of
   * comma-separated items, each a star, a number or a range, with an optional
   * step. Out-of-range and malformed specs match nothing.
   */
  static bool fieldMatches(const pdiutil::string &spec, uint8_t value,
                           uint8_t lo, uint8_t hi);

private:

  /**
   * Writes an empty table carrying only its column names when none is there, so
   * there is always a file to edit rather than one to know how to create.
   */
  static bool ensureTableFile();

  /**
   * Whether every field of a row covers the instant given.
   */
  static bool rowMatches(const pdiutil::string *fields, const datetime_t &now);

  bool m_scanning;
  int16_t m_listener_id;
};

extern CronServiceProvider __cron_service;

#endif

#endif
