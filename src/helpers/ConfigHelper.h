/******************************* Config helper *******************************
This file is part of the pdi stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 1st Aug 2026
******************************************************************************/

#ifndef _CONFIG_HELPER_H_
#define _CONFIG_HELPER_H_

#include <interface/pdi.h>
#include <config/Config.h>
#include <utility/Utility.h>

/* generic config file support functions */

#define CONFIG_TEMP_SUFFIX ".tmp"
#define CONFIG_BOOL_SEPERATOR '|'
#define CONFIG_BOOL_TRUE_VALUES "|yes|true|on|1|"
#define CONFIG_BOOL_FALSE_VALUES "|no|false|off|0|"
#define CONFIG_BOOL_YES "yes"
#define CONFIG_BOOL_NO "no"

bool loadConfigFile(const char *path, pdiutil::vector<config_kv_t> &out);

/**
 * Read a single option out of a config file without holding the rest of it.
 */
bool getConfigValue(const char *path, const char *key, pdiutil::string &out);

/**
 * Persist one option, leaving every other line, comment and ordering intact.
 * The option is appended when the file does not already carry it.
 */
bool setConfigValue(const char *path, const char *key, const char *value);

/**
 * Write a whole config file from key/value pairs, keeping the permissions and
 * ownership the file already had.
 */
bool saveConfigFile(const char *path, const pdiutil::vector<config_kv_t> &kvs, const char *header = nullptr);

/**
 * Create the config file and its directory with the given defaults when it is
 * missing, so a feature always has somewhere to read from.
 */
bool ensureConfigFile(const char *path, const pdiutil::vector<config_kv_t> &defaults, const char *header = nullptr);

/**
 * Read an option's text as a truth value, falling back to the default when it
 * says nothing recognisable.
 */
bool configValueAsBool(const pdiutil::string &value, bool defaultval);

/**
 * Render a truth value the way a config file carries it.
 */
pdiutil::string configBoolAsValue(bool value);

#endif
