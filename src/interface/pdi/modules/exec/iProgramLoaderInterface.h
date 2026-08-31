/************************** Program Loader Interface ***************************
This file is part of the PDI Stack.

This is free software. You can redistribute it and/or modify it but without any
warranty.

Loading a program image and running it, as everything above the port sees it.
What an image is and how it is relocated belongs to the port, so the command,
the scheduler and the task record need no knowledge of any one format.

Author          : Suraj I.
Created Date    : 30th Aug 2026
******************************************************************************/

#ifndef _I_PROGRAM_LOADER_INTERFACE_H_
#define _I_PROGRAM_LOADER_INTERFACE_H_

#include <interface/interface_includes.h>

#ifdef ENABLE_PROGRAM_EXEC

typedef void *program_t;

class iProgramLoaderInterface {

public:
  /**
   * iProgramLoaderInterface constructor.
   */
  iProgramLoaderInterface() {}

  /**
   * iProgramLoaderInterface destructor.
   */
  virtual ~iProgramLoaderInterface() {}

  /**
   * Memory an image can be read into and relocated from, which a port may need
   * to take from somewhere particular.
   */
  virtual void *allocImage(uint32_t _size) = 0;

  /**
   * Release what allocImage returned.
   */
  virtual void freeImage(void *_image) = 0;

  /**
   * Whether the image is one this loader recognises, so a wrong file is
   * refused by name rather than part way through relocation.
   */
  virtual bool isImageValid(const void *_image, uint32_t _size) = 0;

  /**
   * Turn an image into something runnable, returning nullptr and setting _err
   * when it cannot be used. The image stays the caller's to free.
   */
  virtual program_t load(const void *_image, uint32_t _size, int32_t &_err) = 0;

  /**
   * Run the program to completion, occupying the context it is called on.
   */
  virtual int32_t run(program_t _program) = 0;

  /**
   * Release everything load took.
   */
  virtual void unload(program_t _program) = 0;

  /**
   * Stack the program's own context needs, in bytes.
   */
  virtual uint32_t stackSize() const = 0;
};

/**
 * Take the port's loader. Registering none leaves the device without an exec
 * capability, which is a normal state rather than an error.
 */
void registerProgramLoader(iProgramLoaderInterface *_loader);

/**
 * The registered loader, or nullptr when the port has none.
 */
iProgramLoaderInterface *getProgramLoader();

#endif // ENABLE_PROGRAM_EXEC

#endif
