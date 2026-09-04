/******************************* Shell Completion ******************************
This file is part of the pdi stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Completes the word the cursor sits on, the way a shell does it: the first press
inserts as much as every candidate agrees on, and a second press with nothing
left to insert lists what is available. The word is located through the same
tokenizer the parser uses, so the rest of the line is never disturbed and a word
after a pipe or a separator completes as a command rather than as an argument.

Author          : Suraj I.
created Date    : 4th Sep 2026
******************************************************************************/

#ifndef _SHELL_COMPLETION_H_
#define _SHELL_COMPLETION_H_

#include <config/Config.h>

#ifdef ENABLE_CMD_SERVICE

#include <utility/DataTypeDef.h>
#include <utility/iIOInterface.h>

/**
 * What a completion attempt did, so the caller knows whether the line changed
 * and whether the prompt has to be drawn again.
 */
enum shell_complete_result_t : uint8_t {
  SHELL_COMPLETE_NOTHING = 0,
  SHELL_COMPLETE_INSERTED,
  SHELL_COMPLETE_LISTED
};

class ShellCompletion {

public:
  /**
   * Completes the word under the cursor, editing the line in place and writing
   * any candidate list to the terminal.
   */
  static shell_complete_result_t complete(pdiutil::string &line, uint16_t &cursor,
                                          iTerminalInterface *terminal);

private:
  /**
   * Collects the command names that carry this prefix.
   */
  static void gatherCommands(const pdiutil::string &prefix,
                             pdiutil::vector<pdiutil::string> &out);

  /**
   * Collects the service names that carry this prefix.
   */
  static void gatherServices(const pdiutil::string &prefix,
                             pdiutil::vector<pdiutil::string> &out);

  /**
   * Collects the directory entries that carry this prefix, descending into
   * whatever leading directories the word already names.
   */
  static void gatherPaths(const pdiutil::string &word, pdiutil::string &head,
                          pdiutil::vector<pdiutil::string> &out);

  /**
   * What the named command expects at this argument position.
   */
  static cmd_complete_t completionKind(const pdiutil::string &cmdname, uint8_t argindex);

  /**
   * The longest start every candidate shares.
   */
  static pdiutil::string commonPrefix(const pdiutil::vector<pdiutil::string> &items);

  /**
   * Writes the candidates below the line, in columns wide enough for the
   * longest of them.
   */
  static void listCandidates(const pdiutil::vector<pdiutil::string> &items,
                             iTerminalInterface *terminal);
};

#endif

#endif // _SHELL_COMPLETION_H_
