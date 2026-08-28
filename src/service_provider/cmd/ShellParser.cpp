/***************************** Shell Parser ***********************************
This file is part of the pdi stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 26th Aug 2026
******************************************************************************/

#include "ShellParser.h"

#if defined(ENABLE_CMD_SERVICE) && defined(ENABLE_STORAGE_SERVICE)

namespace {

  void trimSpan(const char *line, int16_t &start, int16_t &end) {
    while (start < end && ' ' == line[start]) start++;
    while (end > start && ' ' == line[end - 1]) end--;
  }
}

/**
 * Reads the stages and the output target off a line, reporting an empty
 * stage or a dangling operator as malformed.
 */
ShellParser::Line ShellParser::parse(const char *line, int16_t len) {

  Line out;

  if (nullptr == line || len <= 0) {
    return out;
  }

  int16_t body_end = len;

  int16_t op = -1;
  for (int16_t i = 0; i < len; i++) {
    if ('>' == line[i] || '<' == line[i]) {
      op = i;
      break;
    }
  }

  if (op >= 0) {
    body_end = op;
  }

  // each operator owns the span up to the next one, so one line can name an
  // input and an output in either order
  while (op >= 0 && op < len) {

    bool append = ('>' == line[op]) && ((op + 1) < len) && ('>' == line[op + 1]);
    int16_t pathstart = op + (append ? 2 : 1);

    int16_t next = -1;
    for (int16_t i = pathstart; i < len; i++) {
      if ('>' == line[i] || '<' == line[i]) {
        next = i;
        break;
      }
    }

    int16_t pathend = (next >= 0) ? next : len;
    trimSpan(line, pathstart, pathend);

    if (pathstart >= pathend) {
      out.m_malformed = true;
      return out;
    }

    if ('>' == line[op]) {

      if (out.m_redirected) {
        out.m_malformed = true;
        return out;
      }

      out.m_redirected = true;
      out.m_append = append;
      out.m_outpath.append(line + pathstart, (pdiutil::string::size_type)(pathend - pathstart));
    } else {

      if (out.m_sourced) {
        out.m_malformed = true;
        return out;
      }

      out.m_sourced = true;
      out.m_inpath.append(line + pathstart, (pdiutil::string::size_type)(pathend - pathstart));
    }

    op = next;
  }

  int16_t start = 0;
  for (int16_t i = 0; i <= body_end; i++) {

    if (i == body_end || '|' == line[i]) {

      int16_t s = start;
      int16_t e = i;
      trimSpan(line, s, e);

      if (s >= e) {
        out.m_malformed = true;
        return out;
      }

      Stage stage;
      stage.m_start = s;
      stage.m_len = e - s;
      out.m_stages.push_back(stage);

      start = i + 1;
    }
  }

  if (0 == out.m_stages.size()) {
    out.m_malformed = true;
  }

  return out;
}

#endif
