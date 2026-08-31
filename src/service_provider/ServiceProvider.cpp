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

  _out = CHARPTR_WRAP(SERVICE_CONFIG_DIR_ROOT);
  _out += name;
  _out += FILE_SEPARATOR;
  _out += name;
  _out += CHARPTR_WRAP(SERVICE_CONFIG_FILE_SUFFIX);
}

/**
 * Read the persisted enable state into the service, so later callers answer
 * from memory rather than from storage.
 */
bool ServiceProvider::loadServiceEnabled()
{
#ifdef ENABLE_STORAGE_SERVICE
  pdiutil::string path, value;
  pdiutil::string key = CHARPTR_WRAP(SERVICE_CONFIG_KEY_ENABLED);
  getServiceConfigPath(path);

  if (!path.empty() && getConfigValue(path.c_str(), key.c_str(), value))
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
  pdiutil::string path;
  getServiceConfigPath(path);
  if (path.empty())
  {
    return false;
  }

  pdiutil::string header = CHARPTR_WRAP(SERVICE_CONFIG_HEADER);
  pdiutil::vector<config_kv_t> defaults;
  if (!ensureConfigFile(path.c_str(), defaults, header.c_str()))
  {
    return false;
  }

  pdiutil::string key = CHARPTR_WRAP(SERVICE_CONFIG_KEY_ENABLED);
  pdiutil::string value = configBoolAsValue(_enabled);
  if (!setConfigValue(path.c_str(), key.c_str(), value.c_str()))
  {
    return false;
  }
#endif

  m_service_enabled = _enabled;
  return true;
}
