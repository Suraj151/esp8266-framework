/**************************** Shell Environment *******************************
This file is part of the pdi stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 5th Sep 2026
******************************************************************************/

#include "Environment.h"

#ifdef ENABLE_CMD_SERVICE

#include "SessionManager.h"
#include <utility/DataTypeConversions.h>

#ifdef ENABLE_STORAGE_SERVICE
#include <helpers/ConfigHelper.h>
#endif

namespace {

/**
 * Answers the names the session already knows, so none of them can be stored
 * and go stale.
 */
bool derivedValue(const char *key, pdiutil::string &out)
{
  pdiutil::string name(key);
  out.clear();

  pdiutil::string pwd = CHARPTR_WRAP(ENV_KEY_PWD);
  if (name == pwd) {
#ifdef ENABLE_STORAGE_SERVICE
    out = SessionManager::getPWD();
#endif
    return true;
  }

  pdiutil::string user = CHARPTR_WRAP(ENV_KEY_USER);
  if (name == user) {
#ifdef ENABLE_AUTH_SERVICE
    session_t *s = SessionManager::current();
    if (nullptr != s) out = s->m_username;
#endif
    return true;
  }

  pdiutil::string uid = CHARPTR_WRAP(ENV_KEY_UID);
  if (name == uid) {
    char buf[12];
#ifdef ENABLE_AUTH_SERVICE
    Uint32ToString((uint32_t)SessionManager::getCurrentUid(), buf, sizeof(buf));
#else
    Uint32ToString(0, buf, sizeof(buf));
#endif
    out = buf;
    return true;
  }

  pdiutil::string host = CHARPTR_WRAP(ENV_KEY_HOSTNAME);
  if (name == host) {
#if defined(ENABLE_STORAGE_SERVICE) && defined(ENABLE_NETWORK_SERVICE)
    pdiutil::string path = CHARPTR_WRAP(HOSTNAME_FILE_PATH);
    if (__i_fs.isFileExist(path.c_str())) {
      __i_fs.readLineInFile(path.c_str(), 0, out);
    }
#endif
    return true;
  }

  return false;
}

void appendDerived(pdiutil::vector<config_kv_t> &out, const pdiutil::string &key)
{
  config_kv_t kv;
  kv.m_key = key;
  derivedValue(key.c_str(), kv.m_value);
  out.push_back(kv);
}

int32_t sessionIndexOf(session_t *s, const char *key)
{
  if (nullptr == s || nullptr == key) return -1;

  pdiutil::string name(key);
  for (uint32_t i = 0; i < s->m_env.size(); i++) {
    if (s->m_env[i].m_key == name) return (int32_t)i;
  }

  return -1;
}

}

/**
 * Whether the name is one the session answers for itself and so cannot be
 * assigned.
 */
bool Environment::isDerived(const char *key)
{
  if (nullptr == key || 0 == key[0]) return false;

  pdiutil::string value;
  return derivedValue(key, value);
}

/**
 * Whether the name is one a shell will accept, which is a letter or
 * underscore followed by letters, digits or underscores.
 */
bool Environment::isValidName(const char *key)
{
  if (nullptr == key || 0 == key[0]) return false;

  if (!__is_alpha(key[0]) && '_' != key[0]) return false;

  for (uint16_t i = 1; 0 != key[i]; i++) {
    if (i >= ENV_NAME_MAX) return false;
    if (!__is_alnum(key[i]) && '_' != key[i]) return false;
  }

  return true;
}

/**
 * Value of a variable, taking the first tier that carries it.
 */
bool Environment::get(const char *key, pdiutil::string &out)
{
  if (nullptr == key || 0 == key[0]) return false;

  if (derivedValue(key, out)) return true;

  session_t *s = SessionManager::current();
  int32_t at = sessionIndexOf(s, key);
  if (at >= 0) {
    out = s->m_env[at].m_value;
    return true;
  }

#ifdef ENABLE_STORAGE_SERVICE
  pdiutil::string path = CHARPTR_WRAP(ENV_FILE_PATH);
  if (getConfigValue(path.c_str(), key, out)) return true;
#endif

  out.clear();
  return false;
}

/**
 * Sets a variable for this session only, replacing any value it already had.
 */
pdi_err_t Environment::set(const char *key, const char *value)
{
  if (!isValidName(key)) return CMD_ERROR_INVAL;
  if (isDerived(key)) return CMD_ERROR_PERM;

  session_t *s = SessionManager::current();
  if (nullptr == s) return CMD_ERROR_FAILED;

  int32_t at = sessionIndexOf(s, key);
  if (at >= 0) {
    s->m_env[at].m_value = (nullptr != value) ? value : "";
    return PDI_OK;
  }

  if (s->m_env.size() >= ENV_SESSION_MAX) return CMD_ERROR_FAILED;

  config_kv_t kv;
  kv.m_key = key;
  kv.m_value = (nullptr != value) ? value : "";
  s->m_env.push_back(kv);

  return PDI_OK;
}

/**
 * Drops a variable from this session, leaving the base file alone.
 */
bool Environment::unset(const char *key)
{
  session_t *s = SessionManager::current();
  int32_t at = sessionIndexOf(s, key);
  if (at < 0) return false;

  s->m_env.erase(s->m_env.begin() + at);
  return true;
}

/**
 * Drops every variable this session set, for a login ending or an identity
 * changing on a terminal that outlives both.
 */
void Environment::clearSession()
{
  session_t *s = SessionManager::current();
  if (nullptr == s) return;

  s->m_env.clear();
}

/**
 * Sets a variable in the base file, where every session and the next boot
 * will see it.
 */
pdi_err_t Environment::setPersistent(const char *key, const char *value)
{
  if (!isValidName(key)) return CMD_ERROR_INVAL;
  if (isDerived(key)) return CMD_ERROR_PERM;

#ifdef ENABLE_STORAGE_SERVICE
  ensureBaseFile();

  pdiutil::string path = CHARPTR_WRAP(ENV_FILE_PATH);
  return setConfigValue(path.c_str(), key, (nullptr != value) ? value : "") ?
         PDI_OK : (pdi_err_t)CMD_ERROR_FAILED;
#else
  return PDI_ERR_NOT_SUPPORTED;
#endif
}

/**
 * Drops a variable from the base file.
 */
bool Environment::unsetPersistent(const char *key)
{
#ifdef ENABLE_STORAGE_SERVICE
  if (nullptr == key || 0 == key[0]) return false;

  pdiutil::string path = CHARPTR_WRAP(ENV_FILE_PATH);
  if (!__i_fs.isFileExist(path.c_str())) return false;

  pdiutil::vector<config_kv_t> kvs;
  loadConfigFile(path.c_str(), kvs);

  pdiutil::string name(key);
  bool found = false;

  for (uint32_t i = 0; i < kvs.size(); ) {
    if (kvs[i].m_key == name) {
      kvs.erase(kvs.begin() + i);
      found = true;
    } else {
      i++;
    }
  }

  if (!found) return false;

  return saveConfigFile(path.c_str(), kvs);
#else
  return false;
#endif
}

/**
 * Every variable visible to this session, each from the tier that wins.
 */
void Environment::list(pdiutil::vector<config_kv_t> &out)
{
  out.clear();

  pdiutil::string pwd = CHARPTR_WRAP(ENV_KEY_PWD);
  pdiutil::string user = CHARPTR_WRAP(ENV_KEY_USER);
  pdiutil::string uid = CHARPTR_WRAP(ENV_KEY_UID);
  pdiutil::string host = CHARPTR_WRAP(ENV_KEY_HOSTNAME);

  appendDerived(out, pwd);
  appendDerived(out, user);
  appendDerived(out, uid);
  appendDerived(out, host);

  session_t *s = SessionManager::current();
  if (nullptr != s) {
    for (uint32_t i = 0; i < s->m_env.size(); i++) {
      out.push_back(s->m_env[i]);
    }
  }

#ifdef ENABLE_STORAGE_SERVICE
  pdiutil::string path = CHARPTR_WRAP(ENV_FILE_PATH);
  if (__i_fs.isFileExist(path.c_str())) {

    pdiutil::vector<config_kv_t> base;
    loadConfigFile(path.c_str(), base);

    for (uint32_t i = 0; i < base.size(); i++) {

      bool shadowed = false;
      for (uint32_t j = 0; j < out.size(); j++) {
        if (out[j].m_key == base[i].m_key) { shadowed = true; break; }
      }

      if (!shadowed) out.push_back(base[i]);
    }
  }
#endif
}

#ifdef ENABLE_STORAGE_SERVICE

/**
 * Creates the base environment file with its defaults when it is missing.
 */
void Environment::ensureBaseFile()
{
  pdiutil::string path = CHARPTR_WRAP(ENV_FILE_PATH);
  if (__i_fs.isFileExist(path.c_str())) return;

  pdiutil::string home = CHARPTR_WRAP(ENV_KEY_HOME);
  pdiutil::string root = CHARPTR_WRAP(FILE_SEPARATOR);

  pdiutil::vector<config_kv_t> defaults;
  appendConfigValue(defaults, home, root);

  ensureConfigFile(path.c_str(), defaults);
}

#endif

#endif
