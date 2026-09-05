/**************************** Shell Environment *******************************
This file is part of the pdi stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Variables a shell reads, in three tiers. Facts the session already knows are
answered from it so they cannot go stale; anything set in the session outranks
the base file and ends with the session; the base file is what every session
shares and what survives a reboot.

    lookup(key)
        |
        +-- 1. derived     PWD, USER, UID, HOSTNAME - live, never stored
        +-- 2. session     session_t::m_env, gone when the session ends
        +-- 3. base file   ENV_FILE_PATH, shared and persistent

A board with no filesystem keeps tiers 1 and 2 and simply has no base file.

Author          : Suraj I.
created Date    : 5th Sep 2026
******************************************************************************/

#ifndef _SHELL_ENVIRONMENT_H_
#define _SHELL_ENVIRONMENT_H_

#include <config/Config.h>
#include <utility/DataTypeDef.h>

#ifdef ENABLE_CMD_SERVICE

class Environment {

public:

  /**
   * Value of a variable, taking the first tier that carries it.
   */
  static bool get(const char *key, pdiutil::string &out);

  /**
   * Sets a variable for this session only, replacing any value it already had.
   */
  static pdi_err_t set(const char *key, const char *value);

  /**
   * Drops a variable from this session, leaving the base file alone.
   */
  static bool unset(const char *key);

  /**
   * Drops every variable this session set, for a login ending or an identity
   * changing on a terminal that outlives both.
   */
  static void clearSession();

  /**
   * Sets a variable in the base file, where every session and the next boot
   * will see it.
   */
  static pdi_err_t setPersistent(const char *key, const char *value);

  /**
   * Drops a variable from the base file.
   */
  static bool unsetPersistent(const char *key);

  /**
   * Every variable visible to this session, each from the tier that wins.
   */
  static void list(pdiutil::vector<config_kv_t> &out);

  /**
   * Whether the name is one the session answers for itself and so cannot be
   * assigned.
   */
  static bool isDerived(const char *key);

  /**
   * Whether the name is one a shell will accept, which is a letter or
   * underscore followed by letters, digits or underscores.
   */
  static bool isValidName(const char *key);

#ifdef ENABLE_STORAGE_SERVICE
  /**
   * Creates the base environment file with its defaults when it is missing.
   */
  static void ensureBaseFile();
#endif
};

#endif

#endif
