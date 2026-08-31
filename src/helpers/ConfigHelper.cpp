/******************************* Config helper ********************************
This file is part of the pdi stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 1st Aug 2026
******************************************************************************/

#include <config/Config.h>

#if defined(ENABLE_STORAGE_SERVICE)

#include "ConfigHelper.h"

/* generic config file support functions */

static int readConfigLine(const char *path, uint64_t offset, pdiutil::string &linedata)
{
  linedata.clear();
  int bytes = __i_fs.readFile(path, 128, [&](char *data, uint32_t size) -> bool {
    linedata += pdiutil::string(data, size);
    return true;
  }, offset, "\n");

  if (bytes >= 0 && !linedata.empty() && linedata.back() == '\r')
  {
    linedata.pop_back();
  }

  return bytes;
}

static bool splitConfigLine(const pdiutil::string &linedata, pdiutil::string &key, pdiutil::string &value)
{
  size_t start = 0;
  while (start < linedata.length() && (linedata[start] == ' ' || linedata[start] == '\t')) start++;
  if (start >= linedata.length() || linedata[start] == '#')
  {
    return false;
  }

  size_t kend = start;
  while (kend < linedata.length() && linedata[kend] != ' ' && linedata[kend] != '\t') kend++;

  size_t vstart = kend;
  while (vstart < linedata.length() && (linedata[vstart] == ' ' || linedata[vstart] == '\t')) vstart++;
  size_t vend = linedata.length();
  while (vend > vstart && (linedata[vend - 1] == ' ' || linedata[vend - 1] == '\t')) vend--;

  key = linedata.substr(start, kend - start);
  value = linedata.substr(vstart, vend - vstart);
  return true;
}

static void buildConfigLine(pdiutil::string &out, const char *key, const char *value)
{
  out.clear();
  out += key;
  out += ' ';
  if (nullptr != value)
  {
    out += value;
  }
  out += TERMINAL_NEW_LINE;
}

static void restoreConfigMeta(const char *path, const file_info_t &meta, bool owner)
{
  __i_fs.beginPrivileged();
  __i_fs.setFilePermissions(path, meta.m_perms);
  if (owner)
  {
    __i_fs.setFileOwner(path, meta.m_uid, meta.m_gid);
  }
  __i_fs.endPrivileged();
}

/**
 * @brief Parse a "key value" style config file into key/value pairs.
 *
 * Reads the file line by line. Blank lines and lines starting with '#' are
 * skipped. The first whitespace-separated token is the key, the remainder of
 * the line (leading whitespace trimmed) is the value.
 *
 * @param path Absolute path of the config file.
 * @param out Vector receiving the parsed pairs.
 * @return True if the file exists and was read, false otherwise.
 */
bool loadConfigFile(const char *path, pdiutil::vector<config_kv_t> &out)
{
  if (nullptr == path || !__i_fs.isFileExist(path))
  {
    return false;
  }

  int64_t fs = __i_fs.getFileSize(path);
  if (fs <= 0)
  {
    return false;
  }

  uint64_t offset = 0;
  pdiutil::string linedata;

  while (offset < (uint64_t)fs)
  {
    int bytes = readConfigLine(path, offset, linedata);
    if (bytes < 0) break;

    offset += (uint64_t)bytes + 1;

    config_kv_t kv;
    if (splitConfigLine(linedata, kv.m_key, kv.m_value))
    {
      out.push_back(kv);
    }

    __i_dvc_ctrl.yield();
  }

  return true;
}

/**
 * Read a single option out of a config file without holding the rest of it.
 */
bool getConfigValue(const char *path, const char *key, pdiutil::string &out)
{
  if (nullptr == path || nullptr == key || 0 == key[0] || !__i_fs.isFileExist(path))
  {
    return false;
  }

  int64_t fs = __i_fs.getFileSize(path);
  if (fs <= 0)
  {
    return false;
  }

  uint64_t offset = 0;
  pdiutil::string linedata, linekey, linevalue;

  while (offset < (uint64_t)fs)
  {
    int bytes = readConfigLine(path, offset, linedata);
    if (bytes < 0) break;

    offset += (uint64_t)bytes + 1;

    if (splitConfigLine(linedata, linekey, linevalue) && linekey == key)
    {
      out = linevalue;
      return true;
    }

    __i_dvc_ctrl.yield();
  }

  return false;
}

/**
 * Persist one option, leaving every other line, comment and ordering intact.
 * The option is appended when the file does not already carry it.
 */
bool setConfigValue(const char *path, const char *key, const char *value)
{
  if (nullptr == path || nullptr == key || 0 == key[0])
  {
    return false;
  }

  pdiutil::string newline;
  buildConfigLine(newline, key, value);

  if (!__i_fs.isFileExist(path))
  {
    return __i_fs.createFile(path, newline.c_str(), newline.size()) >= 0;
  }

  file_info_t original;
  bool haveoriginal = (PDI_OK == __i_fs.getFileMeta(path, original));

  pdiutil::string temppath = pdiutil::string(path) + CONFIG_TEMP_SUFFIX;
  if (__i_fs.isFileExist(temppath.c_str()))
  {
    __i_fs.deleteFile(temppath.c_str());
  }
  if (__i_fs.createFile(temppath.c_str(), "", 0) < 0)
  {
    return false;
  }
  if (haveoriginal)
  {
    restoreConfigMeta(temppath.c_str(), original, false);
  }

  int64_t fs = __i_fs.getFileSize(path);
  uint64_t offset = 0;
  bool replaced = false;
  bool writeok = true;
  pdiutil::string linedata, linekey, linevalue, outline;

  while (writeok && fs > 0 && offset < (uint64_t)fs)
  {
    int bytes = readConfigLine(path, offset, linedata);
    if (bytes < 0)
    {
      writeok = false;
      break;
    }

    offset += (uint64_t)bytes + 1;

    if (splitConfigLine(linedata, linekey, linevalue) && linekey == key)
    {
      if (!replaced)
      {
        writeok = (__i_fs.writeFile(temppath.c_str(), newline.c_str(), newline.size(), true) >= 0);
        replaced = true;
      }
    }
    else
    {
      outline = linedata;
      outline += TERMINAL_NEW_LINE;
      writeok = (__i_fs.writeFile(temppath.c_str(), outline.c_str(), outline.size(), true) >= 0);
    }

    __i_dvc_ctrl.yield();
  }

  if (writeok && !replaced)
  {
    writeok = (__i_fs.writeFile(temppath.c_str(), newline.c_str(), newline.size(), true) >= 0);
  }

  if (!writeok)
  {
    __i_fs.deleteFile(temppath.c_str());
    return false;
  }

  __i_fs.deleteFile(path);
  if (PDI_OK != __i_fs.moveFile(temppath.c_str(), path))
  {
    return false;
  }

  if (haveoriginal)
  {
    restoreConfigMeta(path, original, true);
  }

  return true;
}

/**
 * Write a whole config file from key/value pairs, keeping the permissions and
 * ownership the file already had.
 */
bool saveConfigFile(const char *path, const pdiutil::vector<config_kv_t> &kvs, const char *header)
{
  if (nullptr == path)
  {
    return false;
  }

  pdiutil::string content;
  if (nullptr != header)
  {
    content += header;
  }

  pdiutil::string line;
  for (size_t i = 0; i < kvs.size(); i++)
  {
    buildConfigLine(line, kvs[i].m_key.c_str(), kvs[i].m_value.c_str());
    content += line;
    __i_dvc_ctrl.yield();
  }

  file_info_t original;
  bool haveoriginal = (__i_fs.isFileExist(path) && PDI_OK == __i_fs.getFileMeta(path, original));
  if (haveoriginal)
  {
    __i_fs.deleteFile(path);
  }

  if (__i_fs.createFile(path, content.c_str(), content.size()) < 0)
  {
    return false;
  }

  if (haveoriginal)
  {
    restoreConfigMeta(path, original, true);
  }

  return true;
}

/**
 * Create the config file and its directory with the given defaults when it is
 * missing, so a feature always has somewhere to read from.
 */
bool ensureConfigFile(const char *path, const pdiutil::vector<config_kv_t> &defaults, const char *header)
{
  if (nullptr == path)
  {
    return false;
  }

  if (__i_fs.isFileExist(path))
  {
    return true;
  }

  pdiutil::string dir = path;
  pdiutil::string::size_type sep = dir.find_last_of('/');
  if (sep != pdiutil::string::npos && sep > 0)
  {
    dir = dir.substr(0, sep);
    if (!__i_fs.isDirExist(dir.c_str()) && __i_fs.createDirectory(dir.c_str()) < 0)
    {
      return false;
    }
  }

  return saveConfigFile(path, defaults, header);
}

/**
 * Read an option's text as a truth value, falling back to the default when it
 * says nothing recognisable.
 */
bool configValueAsBool(const pdiutil::string &value, bool defaultval)
{
  char buf[8];
  uint8_t len = (uint8_t)value.size();
  if (0 == len || len > (sizeof(buf) - 3))
  {
    return defaultval;
  }

  buf[0] = CONFIG_BOOL_SEPERATOR;
  memcpy(buf + 1, value.c_str(), len);
  buf[len + 1] = CONFIG_BOOL_SEPERATOR;
  buf[len + 2] = '\0';
  __tolowercase(buf, sizeof(buf));

  pdiutil::string truevals = CHARPTR_WRAP(CONFIG_BOOL_TRUE_VALUES);
  if (__strstr(truevals.c_str(), buf, truevals.size()) >= 0)
  {
    return true;
  }

  pdiutil::string falsevals = CHARPTR_WRAP(CONFIG_BOOL_FALSE_VALUES);
  if (__strstr(falsevals.c_str(), buf, falsevals.size()) >= 0)
  {
    return false;
  }

  return defaultval;
}

/**
 * Render a truth value the way a config file carries it.
 */
pdiutil::string configBoolAsValue(bool value)
{
  pdiutil::string out;

  if (value)
  {
    out = CHARPTR_WRAP(CONFIG_BOOL_YES);
  }
  else
  {
    out = CHARPTR_WRAP(CONFIG_BOOL_NO);
  }

  return out;
}

#endif
