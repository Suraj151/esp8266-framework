/***************************** service provider *******************************
This file is part of the pdi stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 1st June 2019
******************************************************************************/

#include "ServiceProvider.h"
#ifdef ENABLE_STORAGE_SERVICE
#include <helpers/ConfigHelper.h>
#endif


// Static member variable to hold the service instances
ServiceProvider *ServiceProvider::m_services[SERVICE_MAX] = {nullptr};

// Static member variable to hold the terminal interface
iTerminalInterface *ServiceProvider::m_terminal = nullptr;

/**
 * Absolute path of the file this service reads its options from.
 */
void ServiceProvider::getServiceConfigPath(pdiutil::string &_out)
{
  _out.clear();

#ifdef ENABLE_STORAGE_SERVICE
  pdiutil::string name;
  getServiceConfigName(name);

  if (!name.empty())
  {
    buildConfigPath(name.c_str(), _out);
  }
#endif
}

/**
 * The service's own name in the form its config carries, which is the key the
 * enable state is listed under and the stem of its settings file.
 */
void ServiceProvider::getServiceConfigName(pdiutil::string &_out)
{
  _out.clear();

  if (nullptr == m_service_name)
  {
    return;
  }

  char name[SERVICE_NAME_MAX];
  uint32_t len = (uint32_t)strlen_ro(m_service_name);
  if (len >= sizeof(name))
  {
    len = sizeof(name) - 1;
  }
  memcpy_ro(name, m_service_name, len);
  name[len] = '\0';
  __tolowercase(name, sizeof(name));

  _out = name;
}

/**
 * Read the persisted enable state into the service, so later callers answer
 * from memory rather than from storage.
 */
bool ServiceProvider::loadServiceEnabled()
{
#ifdef ENABLE_STORAGE_SERVICE
  pdiutil::string key, value;
  pdiutil::string path = CHARPTR_WRAP(SERVICE_ENABLE_CONFIG_FILE);
  getServiceConfigName(key);

  if (!key.empty() && getConfigValue(path.c_str(), key.c_str(), value))
  {
    m_service_enabled = configValueAsBool(value, true);
  }
#endif

  return isServiceEnabled();
}

/**
 * Persist whether this service is meant to run, so the choice survives a
 * restart. An essential service refuses to be turned off.
 */
bool ServiceProvider::setServiceEnabled(bool _enabled)
{
  if (!_enabled && isEssentialService())
  {
    return false;
  }

#ifdef ENABLE_STORAGE_SERVICE
  pdiutil::string key;
  getServiceConfigName(key);
  if (key.empty())
  {
    return false;
  }

  pdiutil::string path = CHARPTR_WRAP(SERVICE_ENABLE_CONFIG_FILE);
  pdiutil::string header = CHARPTR_WRAP(SERVICE_ENABLE_CONFIG_HEADER);
  pdiutil::vector<config_kv_t> defaults;
  if (!ensureConfigFile(path.c_str(), defaults, header.c_str()))
  {
    return false;
  }

  pdiutil::string value = configBoolAsValue(_enabled);
  if (!setConfigValue(path.c_str(), key.c_str(), value.c_str()))
  {
    return false;
  }
#endif

  m_service_enabled = _enabled;
  return true;
}
