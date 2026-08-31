/************************** Program Loader Interface ***************************
This file is part of the pdi stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 30th Aug 2026
******************************************************************************/

#ifndef _PDI_ESP32_PROGRAM_LOADER_INTERFACE_H_
#define _PDI_ESP32_PROGRAM_LOADER_INTERFACE_H_

#include "esp32.h"
#include <interface/interface_includes.h>

#ifdef ENABLE_PROGRAM_EXEC

#include <interface/pdi/modules/exec/iProgramLoaderInterface.h>

class ProgramLoaderInterface : public iProgramLoaderInterface
{

public:
  /**
   * ProgramLoaderInterface constructor.
   */
  ProgramLoaderInterface() {}

  /**
   * ProgramLoaderInterface destructor.
   */
  ~ProgramLoaderInterface() {}

  /**
   * Memory an image can be read into and relocated from.
   */
  void *allocImage(uint32_t _size) override;

  /**
   * Release what allocImage returned.
   */
  void freeImage(void *_image) override;

  /**
   * The four byte magic, checked before anything is relocated.
   */
  bool isImageValid(const void *_image, uint32_t _size) override;

  /**
   * Relocate an image into a runnable object.
   */
  program_t load(const void *_image, uint32_t _size, int32_t &_err) override;

  /**
   * Run the program to completion on the calling context.
   */
  int32_t run(program_t _program) override;

  /**
   * Release everything load took.
   */
  void unload(program_t _program) override;

  /**
   * Stack the program's own context needs.
   */
  uint32_t stackSize() const override;
};

extern ProgramLoaderInterface __i_program_loader;

#endif // ENABLE_PROGRAM_EXEC

#endif // _PDI_ESP32_PROGRAM_LOADER_INTERFACE_H_
