/******************************* Shell Completion ******************************
This file is part of the pdi stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 4th Sep 2026
******************************************************************************/

#include "ShellCompletion.h"

#ifdef ENABLE_CMD_SERVICE

#include <utility/CommandBase.h>
#include <service_provider/ServiceProvider.h>
#include <interface/pdi.h>

#ifdef ENABLE_STORAGE_SERVICE
#include "ShellParser.h"
#include <service_provider/session/SessionManager.h>
#endif

namespace {

const uint8_t COMPLETION_COLUMN_GAP = 2;
const uint8_t COMPLETION_MIN_COLUMN = 8;
const uint8_t COMPLETION_LINE_WIDTH = 72;

#ifdef ENABLE_STORAGE_SERVICE

bool startsACommand(const pdiutil::vector<ShellParser::Token> &tokens, uint16_t index)
{
  if (0 == index) return true;

  switch (tokens[index - 1].m_type) {
    case ShellParser::TOKEN_PIPE:
    case ShellParser::TOKEN_AND_IF:
    case ShellParser::TOKEN_OR_IF:
    case ShellParser::TOKEN_SEMI:
      return true;
    default:
      return false;
  }
}

bool namesARedirect(const pdiutil::vector<ShellParser::Token> &tokens, uint16_t index)
{
  if (0 == index) return false;

  switch (tokens[index - 1].m_type) {
    case ShellParser::TOKEN_LT:
    case ShellParser::TOKEN_GT:
    case ShellParser::TOKEN_DGT:
      return true;
    default:
      return false;
  }
}

#endif

}

/**
 * Collects the command names that carry this prefix.
 */
void ShellCompletion::gatherCommands(const pdiutil::string &prefix,
                                     pdiutil::vector<pdiutil::string> &out)
{
  for (uint16_t i = 0; i < CommandBase::CommandRegistry().size(); i++) {

    const char *name = CommandBase::CommandRegistry()[i].cmdname;
    if (nullptr == name) continue;

    if (!prefix.empty() && !CommandBase::isCommandMatch(name, prefix.c_str(), true)) {
      continue;
    }

    char buf[CMD_SIZE_MAX];
    memset(buf, 0, CMD_SIZE_MAX);
    uint32_t len = strlen_ro(name);
    if (len > CMD_SIZE_MAX - 1) len = CMD_SIZE_MAX - 1;
    memcpy_ro(buf, name, len);

    out.push_back(pdiutil::string(buf));
  }
}

/**
 * Collects the service names that carry this prefix.
 */
void ShellCompletion::gatherServices(const pdiutil::string &prefix,
                                     pdiutil::vector<pdiutil::string> &out)
{
  for (uint8_t i = 0; i < SERVICE_MAX; i++) {

    ServiceProvider *s = ServiceProvider::getService((service_t)i);
    if (nullptr == s || nullptr == s->m_service_name) continue;

    char buf[SERVICE_NAME_MAX];
    memset(buf, 0, SERVICE_NAME_MAX);
    uint32_t len = strlen_ro(s->m_service_name);
    if (len > SERVICE_NAME_MAX - 1) len = SERVICE_NAME_MAX - 1;
    memcpy_ro(buf, s->m_service_name, len);

    if (prefix.size() > len) continue;
    if (0 != strncmp(buf, prefix.c_str(), prefix.size())) continue;

    out.push_back(pdiutil::string(buf));
  }
}

/**
 * Collects the directory entries that carry this prefix, descending into
 * whatever leading directories the word already names.
 */
void ShellCompletion::gatherPaths(const pdiutil::string &word, pdiutil::string &head,
                                  pdiutil::vector<pdiutil::string> &out)
{
  head.clear();

#ifdef ENABLE_STORAGE_SERVICE

  pdiutil::string separator = CHARPTR_WRAP(FILE_SEPARATOR);
  pdiutil::string dir;
  pdiutil::string base = word;

  pdiutil::string::size_type cut = word.find_last_of(separator);

  if (cut != pdiutil::string::npos) {
    head = word.substr(0, cut + 1);
    base = word.substr(cut + 1);
    dir = (0 == cut) ? separator : word.substr(0, cut);
  }

  if (dir.empty()) {
    dir = SessionManager::getPWD();
  } else if (separator[0] != dir[0]) {
    pdiutil::string pwd = SessionManager::getPWD();
    if (!pwd.empty() && separator[0] != pwd[pwd.size() - 1]) pwd += separator;
    dir = pwd + dir;
  }

  pdiutil::vector<file_info_t> items;
  int result = __i_fs.getDirFileList(dir.c_str(), items, base.c_str());

  if (result >= 0) {
    for (file_info_t &item : items) {
      if (nullptr == item.m_name) continue;
      pdiutil::string entry(item.m_name);
      if (FILE_TYPE_DIR == item.m_type) entry += separator;
      out.push_back(entry);
    }
  }

  for (file_info_t &item : items) {
    pdiutil::safe_delete_array(item.m_name);
  }
  items.clear();

#endif
}

/**
 * What the named command expects at this argument position.
 */
cmd_complete_t ShellCompletion::completionKind(const pdiutil::string &cmdname, uint8_t argindex)
{
  cmd_complete_t kind = CMD_COMPLETE_PATH;

  CommandBase *cmd = CommandBase::GetCommand(cmdname.c_str());
  if (nullptr != cmd) {
    kind = cmd->completionFor(argindex);
    pdiutil::safe_delete(cmd);
  }

  return kind;
}

/**
 * The longest start every candidate shares.
 */
pdiutil::string ShellCompletion::commonPrefix(const pdiutil::vector<pdiutil::string> &items)
{
  if (0 == items.size()) return pdiutil::string();

  pdiutil::string prefix = items[0];

  for (uint32_t i = 1; i < items.size(); i++) {

    uint32_t keep = 0;
    while (keep < prefix.size() && keep < items[i].size() && prefix[keep] == items[i][keep]) {
      keep++;
    }
    prefix = prefix.substr(0, keep);

    if (prefix.empty()) break;
  }

  return prefix;
}

/**
 * Writes the candidates below the line, in columns wide enough for the
 * longest of them.
 */
void ShellCompletion::listCandidates(const pdiutil::vector<pdiutil::string> &items,
                                     iTerminalInterface *terminal)
{
  uint32_t widest = 0;
  for (uint32_t i = 0; i < items.size(); i++) {
    if (items[i].size() > widest) widest = items[i].size();
  }

  uint32_t column = widest + COMPLETION_COLUMN_GAP;
  if (column < COMPLETION_MIN_COLUMN) column = COMPLETION_MIN_COLUMN;

  uint32_t percol = COMPLETION_LINE_WIDTH / column;
  if (0 == percol) percol = 1;

  terminal->putln();

  for (uint32_t i = 0; i < items.size(); i++) {

    bool endofrow = (percol - 1 == (i % percol)) || (items.size() - 1 == i);

    if (endofrow) {
      terminal->writeln(items[i].c_str());
    } else {
      terminal->write_pad(items[i].c_str(), column);
    }
  }
}

/**
 * Completes the word under the cursor, editing the line in place and writing
 * any candidate list to the terminal.
 */
shell_complete_result_t ShellCompletion::complete(pdiutil::string &line, uint16_t &cursor,
                                                  iTerminalInterface *terminal)
{
  // an empty line asks for every command there is, which is a page of them on
  // a console with nothing to page it. nothing typed, nothing offered
  if (nullptr == terminal || line.empty()) return SHELL_COMPLETE_NOTHING;

  if (cursor > (uint16_t)line.size()) cursor = (uint16_t)line.size();

  pdiutil::vector<pdiutil::string> candidates;
  pdiutil::string word;
  pdiutil::string head;

#ifdef ENABLE_STORAGE_SERVICE

  pdiutil::vector<ShellParser::Token> tokens;
  ShellParser::tokenize(line.c_str(), (int16_t)line.size(), tokens);

  // the word being completed is the one the cursor sits in or ends. a cursor
  // past every word starts a fresh one where it stands
  int32_t wordat = -1;
  uint16_t insertat = tokens.size();

  for (uint16_t i = 0; i < tokens.size(); i++) {

    if (ShellParser::TOKEN_WORD != tokens[i].m_type) {
      if (tokens[i].m_start >= (int16_t)cursor) { insertat = i; break; }
      continue;
    }

    if ((int16_t)cursor >= tokens[i].m_start &&
        (int16_t)cursor <= tokens[i].m_start + tokens[i].m_len) {
      wordat = (int32_t)i;
      insertat = i;
      break;
    }

    if (tokens[i].m_start > (int16_t)cursor) { insertat = i; break; }
  }

  if (wordat >= 0) {
    uint16_t wordstart = (uint16_t)tokens[wordat].m_start;
    word = line.substr(wordstart, cursor - wordstart);
  }

  uint16_t position = (wordat >= 0) ? (uint16_t)wordat : insertat;

  if (namesARedirect(tokens, position)) {

    gatherPaths(word, head, candidates);

  } else if (startsACommand(tokens, position)) {

    gatherCommands(word, candidates);

  } else {

    // the nearest word to the left that follows no other word is the command,
    // and the distance to it is which argument this is
    int32_t cmdat = -1;
    uint8_t argindex = 0;

    for (int32_t i = (int32_t)position - 1; i >= 0; i--) {
      if (ShellParser::TOKEN_WORD != tokens[i].m_type) break;
      cmdat = i;
      argindex++;
    }

    if (cmdat < 0) return SHELL_COMPLETE_NOTHING;

    pdiutil::string cmdname = line.substr(tokens[cmdat].m_start, tokens[cmdat].m_len);
    cmd_complete_t kind = completionKind(cmdname, (uint8_t)(argindex - 1));

    switch (kind) {
      case CMD_COMPLETE_SERVICE: gatherServices(word, candidates); break;
      case CMD_COMPLETE_NONE:    return SHELL_COMPLETE_NOTHING;
      default:                   gatherPaths(word, head, candidates); break;
    }
  }

#else

  // without a filesystem there is no grammar to read the line with and nothing
  // but command names to offer, so a line that has reached its arguments is
  // as complete as it can be made
  if (line.find(CMD_OPTION_SEPERATOR_SPACE) != pdiutil::string::npos) {
    return SHELL_COMPLETE_NOTHING;
  }

  word = line;
  cursor = (uint16_t)line.size();
  gatherCommands(word, candidates);

#endif

  if (0 == candidates.size()) return SHELL_COMPLETE_NOTHING;

  // whatever every candidate agrees on can be typed for the user. the word
  // already carries its leading directories, so only the tail is compared
  pdiutil::string tail = word.substr(head.size());
  pdiutil::string shared = commonPrefix(candidates);

  if (shared.size() > tail.size()) {

    pdiutil::string addition = shared.substr(tail.size());

    if (1 == candidates.size() && FILE_SEPARATOR[0] != shared[shared.size() - 1]) {
      addition += CMD_OPTION_SEPERATOR_SPACE;
    }

    line.insert(cursor, addition.c_str());
    cursor += (uint16_t)addition.size();
    terminal->write(addition.c_str());

    return SHELL_COMPLETE_INSERTED;
  }

  // nothing more can be typed, so show what is on offer instead
  if (candidates.size() > 1) {
    listCandidates(candidates, terminal);
    return SHELL_COMPLETE_LISTED;
  }

  return SHELL_COMPLETE_NOTHING;
}

#endif
