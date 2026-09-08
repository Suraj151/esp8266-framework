/**************************** Null Terminal ***********************************
This file is part of the pdi stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

A terminal that swallows everything written to it and never has anything to
read. Work that has to run under a session but has nobody watching attaches to
this instead of the console, so it neither borrows a terminal somebody is
logged in on nor waits for one to be free.

Author          : Suraj I.
created Date    : 8th Sep 2026
******************************************************************************/

#ifndef _NULL_TERMINAL_H_
#define _NULL_TERMINAL_H_

#include <utility/iIOInterface.h>

class NullTerminal : public iTerminalInterface
{

public:

  NullTerminal();
  ~NullTerminal();

  using iTerminalInterface::write;
  using iTerminalInterface::read;

  int16_t disconnect() override;
  int8_t connected() override;

  int32_t write(uint8_t c) override;
  int32_t write(const uint8_t *c_str) override;
  int32_t write(const uint8_t *c_str, uint32_t size) override;

  uint8_t read() override;
  int32_t read(uint8_t *buf, uint32_t size) override;
  int32_t available() override;
};

extern NullTerminal __i_null_terminal;

#endif
