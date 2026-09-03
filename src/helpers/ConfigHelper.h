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
 * Build the config file path a feature is read from, so the service layer and
 * the feature tables cannot drift on where a config lives.
 */
void buildConfigPath(const char *name, pdiutil::string &out);

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
 * ownership the file already had. A file being created takes the given mode,
 * owned by root, so one carrying a secret is not left world readable.
 */
bool saveConfigFile(const char *path, const pdiutil::vector<config_kv_t> &kvs, const char *header = nullptr, uint16_t mode = 0);

/**
 * Create the config file and its directory with the given defaults when it is
 * missing, so a feature always has somewhere to read from.
 */
bool ensureConfigFile(const char *path, const pdiutil::vector<config_kv_t> &defaults, const char *header = nullptr, uint16_t mode = 0);

/**
 * Bring the file to the given options, creating it when absent and otherwise
 * rewriting only the ones whose value actually changed.
 */
bool writeConfigValues(const char *path, const pdiutil::vector<config_kv_t> &kvs, const char *header = nullptr, uint16_t mode = 0);

/**
 * Add an option to a set being built, so a caller renders its settings without
 * knowing how a config line is spelled.
 */
void appendConfigValue(pdiutil::vector<config_kv_t> &kvs, const pdiutil::string &key, const pdiutil::string &value);

/**
 * Read one option out of a set already in hand, for a caller that would
 * otherwise walk the whole file once per key.
 */
bool findConfigValue(const pdiutil::vector<config_kv_t> &kvs, const pdiutil::string &key, pdiutil::string &out);

/**
 * Take an option into a fixed width text field, leaving the field alone when
 * the set does not carry the option or the value would not fit.
 */
bool takeConfigText(const pdiutil::vector<config_kv_t> &kvs, const pdiutil::string &key, char *field, uint16_t size);

/**
 * Take an option into a four octet address, leaving the field alone unless the
 * value reads back as the very address it spells.
 */
bool takeConfigAddress(const pdiutil::vector<config_kv_t> &kvs, const pdiutil::string &key, uint8_t *field);

/**
 * Take an option into a truth value, leaving the field alone when the set does
 * not carry the option or its text says nothing recognisable.
 */
bool takeConfigBool(const pdiutil::vector<config_kv_t> &kvs, const pdiutil::string &key, bool *field);

/**
 * Take an option into a truth value kept as a byte, for a record that spells a
 * flag as a number rather than as a bool.
 */
bool takeConfigBool(const pdiutil::vector<config_kv_t> &kvs, const pdiutil::string &key, uint8_t *field);

/**
 * Take an option into a whole number, leaving the field alone unless every
 * character is a digit and the result is one the field is allowed to hold.
 */
bool takeConfigNumber(const pdiutil::vector<config_kv_t> &kvs, const pdiutil::string &key, uint8_t *field, uint8_t maxvalue = 0xFF);
bool takeConfigNumber(const pdiutil::vector<config_kv_t> &kvs, const pdiutil::string &key, uint16_t *field, uint16_t maxvalue = 0xFFFF);

/**
 * Render a four octet address the way a config file carries it.
 */
pdiutil::string configAddressAsValue(const uint8_t *octets);

/**
 * Read an option's text as a truth value, falling back to the default when it
 * says nothing recognisable.
 */
bool configValueAsBool(const pdiutil::string &value, bool defaultval);

/**
 * Render a truth value the way a config file carries it.
 */
pdiutil::string configBoolAsValue(bool value);

/**
 * Render a whole number the way a config file carries it.
 */
pdiutil::string configNumberAsValue(uint32_t value);

#endif
