/***************************** Shell Parser ***********************************
This file is part of the pdi stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Splits a typed line into the stages of a pipeline and the file its output should
land in, so the shell can claim descriptors before anything runs and no command
has to know it was redirected or piped.

Author          : Suraj I.
created Date    : 26th Aug 2026
******************************************************************************/
#ifndef _SHELL_PARSER_H_
#define _SHELL_PARSER_H_

#include <config/Config.h>
#include <utility/DataTypeDef.h>

#if defined(ENABLE_CMD_SERVICE) && defined(ENABLE_STORAGE_SERVICE)

class ShellParser {

public:

  struct Stage {

    Stage() : m_start(0), m_len(0) {}

    int16_t m_start;
    int16_t m_len;
  };

  struct Line {

    Line() : m_append(false), m_redirected(false), m_sourced(false),
             m_malformed(false) {}

    /**
     * Whether the line needs descriptors at all, which a plain command
     * with one stage and neither a source nor a target does not.
     */
    bool isPlain() const {
      return m_stages.size() < 2 && !m_redirected && !m_sourced;
    }

    pdiutil::vector<Stage> m_stages;
    pdiutil::string m_outpath;
    pdiutil::string m_inpath;
    bool m_append;
    bool m_redirected;
    bool m_sourced;
    bool m_malformed;
  };

  /**
   * Reads the stages and the output target off a line, reporting an empty
   * stage or a dangling operator as malformed.
   */
  static Line parse(const char *line, int16_t len);
};

#endif

#endif
