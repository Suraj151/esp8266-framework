/***************************** Shell Parser ***********************************
This file is part of the pdi stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 26th Aug 2026
******************************************************************************/

#include "ShellParser.h"
#include <utility/DataTypeConversions.h>
#include <utility/StringOperations.h>
#include <service_provider/session/Environment.h>

#if defined(ENABLE_CMD_SERVICE) && defined(ENABLE_STORAGE_SERVICE)

namespace {

  bool isBlank(char c) {
    return ' ' == c || '\t' == c;
  }

  // Where an operator starts, so a word knows where to end.
  bool isOperatorStart(char c) {
    return '|' == c || '&' == c || ';' == c || '<' == c || '>' == c;
  }
}

/**
 * Splits a line into tokens, honouring quotes and backslash escapes.
 * False when a quote is never closed.
 */
bool ShellParser::tokenize(const char *line, int16_t len, pdiutil::vector<Token> &tokens) {

  tokens.clear();

  if (nullptr == line || len <= 0) {
    return false;
  }

  int16_t i = 0;

  while (i < len) {

    while (i < len && isBlank(line[i])) i++;
    if (i >= len) break;

    Token token;
    token.m_start = i;

    // operators first, so a word never starts on one
    if (isOperatorStart(line[i])) {

      char c = line[i];
      bool doubled = ((i + 1) < len) && (line[i + 1] == c);

      if ('|' == c) {
        token.m_type = doubled ? TOKEN_OR_IF : TOKEN_PIPE;
      } else if ('&' == c) {
        // a lone '&' is not an operator here, so it stays part of a word
        if (!doubled) {
          token.m_type = TOKEN_WORD;
        } else {
          token.m_type = TOKEN_AND_IF;
        }
      } else if (';' == c) {
        token.m_type = TOKEN_SEMI;
      } else if ('>' == c) {
        token.m_type = doubled ? TOKEN_DGT : TOKEN_GT;
      } else {
        token.m_type = TOKEN_LT;
      }

      if (TOKEN_WORD != token.m_type) {
        token.m_len = (doubled && (TOKEN_SEMI != token.m_type)) ? 2 : 1;
        tokens.push_back(token);
        i += token.m_len;
        continue;
      }
    }

    // a word runs to the next blank or operator, with quotes and escapes
    // holding those characters inside it
    while (i < len) {

      char c = line[i];

      if (isBlank(c)) break;

      if ('\\' == c) {
        i += ((i + 1) < len) ? 2 : 1;
        continue;
      }

      if ('\'' == c || '"' == c) {

        char quote = c;
        i++;

        while (i < len && line[i] != quote) {
          if ('"' == quote && '\\' == line[i] && (i + 1) < len) {
            i += 2;
            continue;
          }
          i++;
        }

        if (i >= len) {
          tokens.clear();
          return false;
        }

        i++;
        continue;
      }

      // a lone '&' belongs to the word; a doubled one ends it
      if ('&' == c) {
        if ((i + 1) < len && '&' == line[i + 1]) break;
        i++;
        continue;
      }

      if (isOperatorStart(c)) break;

      i++;
    }

    token.m_type = TOKEN_WORD;
    token.m_len = i - token.m_start;

    if (token.m_len > 0) {
      tokens.push_back(token);
    }
  }

  return tokens.size() > 0;
}

/**
 * Whether a span holds anything the expander would rewrite.
 */
bool ShellParser::expandable(const char *src, int16_t len) {

  for (int16_t i = 0; i < len; i++) {
    if ('$' == src[i]) {
      return true;
    }
  }

  return false;
}

/**
 * Copies a span with expansions applied. A single quoted run is literal, so
 * nothing inside it expands; a double quoted one still does.
 */
void ShellParser::expand(const char *src, int16_t len, pdi_err_t lastexit, pdiutil::string &out,
                         bool unquote) {

  char number[12] = {0};
  bool rendered = false;
  bool insingle = false;
  bool indouble = false;

  for (int16_t i = 0; i < len; i++) {

    char c = src[i];

    if ('\\' == c && !insingle && (i + 1) < len) {
      if (!unquote) out += c;
      out += src[i + 1];
      i++;
      continue;
    }

    if ('\'' == c && !indouble) {
      insingle = !insingle;
      if (!unquote) out += c;
      continue;
    }

    if ('"' == c && !insingle) {
      indouble = !indouble;
      if (!unquote) out += c;
      continue;
    }

    if ('$' == c && !insingle && (i + 1) < len && '?' == src[i + 1]) {

      if (!rendered) {
        Int32ToString((int32_t)lastexit, number, sizeof(number) - 1);
        rendered = true;
      }

      out += number;
      i++;
      continue;
    }

    // a name after $ is a variable, and one the environment does not carry
    // expands to nothing the way a shell leaves an unset name empty
    if ('$' == c && !insingle && (i + 1) < len &&
        (__is_alpha(src[i + 1]) || '_' == src[i + 1])) {

      int16_t nameend = i + 1;
      while (nameend < len && (__is_alnum(src[nameend]) || '_' == src[nameend])) nameend++;

      pdiutil::string name(src + i + 1, nameend - (i + 1));
      pdiutil::string value;
      Environment::get(name.c_str(), value);

      out += value;
      i = nameend - 1;
      continue;
    }

    out += c;
  }
}

/**
 * Text of one command without its redirections, expansions applied.
 */
bool ShellParser::Line::commandText(const char *line, int16_t index, pdiutil::string &out,
                                    pdi_err_t lastexit) const {

  out.clear();

  if (nullptr == line || index < 0 || index >= (int16_t)m_commands.size()) {
    return false;
  }

  const Command &command = m_commands[index];
  if (command.m_word_count <= 0) {
    return false;
  }

  int16_t first = m_words[command.m_word_start].m_start;
  const Token &lastword = m_words[command.m_word_start + command.m_word_count - 1];
  int16_t last = lastword.m_start + lastword.m_len;

  // nothing of the command's own redirections falls between its first and last
  // word in the common case, so the line can be handed over as it was typed
  bool contiguous = true;
  for (int16_t r = 0; r < command.m_redirect_count; r++) {
    const Redirect &redirect = m_redirects[command.m_redirect_start + r];
    if (redirect.m_start > first && redirect.m_start < last) {
      contiguous = false;
      break;
    }
  }

  if (contiguous && !expandable(line + first, last - first)) {
    out.append(line + first, (pdiutil::string::size_type)(last - first));
    return true;
  }

  if (contiguous) {
    expand(line + first, last - first, lastexit, out);
    return true;
  }

  for (int16_t w = 0; w < command.m_word_count; w++) {
    const Token &word = m_words[command.m_word_start + w];
    if (w > 0) out += ' ';
    expand(line + word.m_start, word.m_len, lastexit, out);
  }

  return true;
}

/**
 * Redirection of the given kind on one command, if it has one.
 */
const ShellParser::Redirect *ShellParser::Line::findRedirect(int16_t command, redirect_op_t op) const {

  if (command < 0 || command >= (int16_t)m_commands.size()) {
    return nullptr;
  }

  const Command &c = m_commands[command];

  for (int16_t r = 0; r < c.m_redirect_count; r++) {
    const Redirect &redirect = m_redirects[c.m_redirect_start + r];
    if (redirect.m_op == op) {
      return &redirect;
    }
  }

  return nullptr;
}

/**
 * Reads a whole line into pipelines, commands and redirections.
 */
ShellParser::Line ShellParser::parse(const char *line, int16_t len) {

  Line out;

  pdiutil::vector<Token> tokens;
  if (!tokenize(line, len, tokens)) {
    out.m_malformed = true;
    return out;
  }

  uint16_t at = 0;
  join_t join = JOIN_FIRST;

  while (at < tokens.size()) {

    Pipeline pipeline;
    pipeline.m_join = join;
    pipeline.m_command_start = (int16_t)out.m_commands.size();
    pipeline.m_command_count = 0;

    // a pipeline is one or more commands joined by '|'
    while (true) {

      Command command;
      command.m_word_start = (int16_t)out.m_words.size();
      command.m_redirect_start = (int16_t)out.m_redirects.size();
      command.m_word_count = 0;
      command.m_redirect_count = 0;

      while (at < tokens.size()) {

        const Token &token = tokens[at];

        if (TOKEN_WORD == token.m_type) {
          out.m_words.push_back(token);
          command.m_word_count++;
          at++;
          continue;
        }

        if (TOKEN_LT == token.m_type || TOKEN_GT == token.m_type || TOKEN_DGT == token.m_type) {

          // a redirection needs a path, and only one of each kind per command
          if ((at + 1) >= tokens.size() || TOKEN_WORD != tokens[at + 1].m_type) {
            out.m_malformed = true;
            return out;
          }

          Redirect redirect;
          redirect.m_op = (TOKEN_LT == token.m_type) ? REDIRECT_IN
                        : (TOKEN_DGT == token.m_type) ? REDIRECT_APPEND : REDIRECT_OUT;
          redirect.m_start = tokens[at + 1].m_start;
          redirect.m_len = tokens[at + 1].m_len;

          for (int16_t r = 0; r < command.m_redirect_count; r++) {
            const Redirect &seen = out.m_redirects[command.m_redirect_start + r];
            bool bothOut = (REDIRECT_IN != seen.m_op) && (REDIRECT_IN != redirect.m_op);
            if (seen.m_op == redirect.m_op || bothOut) {
              out.m_malformed = true;
              return out;
            }
          }

          out.m_redirects.push_back(redirect);
          command.m_redirect_count++;
          at += 2;
          continue;
        }

        break;
      }

      if (0 == command.m_word_count) {
        out.m_malformed = true;
        return out;
      }

      out.m_commands.push_back(command);
      pipeline.m_command_count++;

      if (at < tokens.size() && TOKEN_PIPE == tokens[at].m_type) {
        at++;
        continue;
      }

      break;
    }

    out.m_pipelines.push_back(pipeline);

    if (at >= tokens.size()) {
      break;
    }

    if (TOKEN_SEMI == tokens[at].m_type) {
      join = JOIN_ALWAYS;
    } else if (TOKEN_AND_IF == tokens[at].m_type) {
      join = JOIN_ON_SUCCESS;
    } else if (TOKEN_OR_IF == tokens[at].m_type) {
      join = JOIN_ON_FAILURE;
    } else {
      out.m_malformed = true;
      return out;
    }

    at++;

    // a trailing ';' ends the line, anything else is left dangling
    if (at >= tokens.size()) {
      if (JOIN_ALWAYS != join) {
        out.m_malformed = true;
      }
      return out;
    }
  }

  if (0 == out.m_pipelines.size()) {
    out.m_malformed = true;
  }

  return out;
}

/**
 * Whether a pipeline runs, given what the one before it answered.
 */
bool ShellParser::shouldRun(join_t join, pdi_err_t previous) {

  if (JOIN_ON_SUCCESS == join) {
    return PDI_OK == previous;
  }

  if (JOIN_ON_FAILURE == join) {
    return PDI_OK != previous;
  }

  return true;
}

#endif
