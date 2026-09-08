/**************************** Null Terminal ***********************************
This file is part of the pdi stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 8th Sep 2026
******************************************************************************/

#include "NullTerminal.h"

NullTerminal __i_null_terminal;

NullTerminal::NullTerminal()
{
}

NullTerminal::~NullTerminal()
{
}

int16_t NullTerminal::disconnect()
{
  return 0;
}

int8_t NullTerminal::connected()
{
  return 1;
}

int32_t NullTerminal::write(uint8_t c)
{
  return 1;
}

int32_t NullTerminal::write(const uint8_t *c_str)
{
  if( nullptr == c_str ){
    return 0;
  }

  return (int32_t)strlen((const char *)c_str);
}

int32_t NullTerminal::write(const uint8_t *c_str, uint32_t size)
{
  if( nullptr == c_str ){
    return 0;
  }

  return (int32_t)size;
}

uint8_t NullTerminal::read()
{
  return 0;
}

int32_t NullTerminal::read(uint8_t *buf, uint32_t size)
{
  return 0;
}

int32_t NullTerminal::available()
{
  return 0;
}
