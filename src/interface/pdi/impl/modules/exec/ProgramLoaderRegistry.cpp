/************************** Program Loader Registry ****************************
This file is part of the PDI Stack.

This is free software. You can redistribute it and/or modify it but without any
warranty.

One slot, because a device has one way of loading a program. It is kept here
rather than as a port global so that everything above can ask whether the
capability exists at all without naming a port.

Author          : Suraj I.
Created Date    : 30th Aug 2026
******************************************************************************/

#include <config/Config.h>

#ifdef ENABLE_PROGRAM_EXEC

#include <interface/pdi/modules/exec/iProgramLoaderInterface.h>

namespace {
iProgramLoaderInterface *s_program_loader = nullptr;
}

/**
 * Take the port's loader, from its own init.
 */
void registerProgramLoader(iProgramLoaderInterface *_loader) {
    s_program_loader = _loader;
}

/**
 * The registered loader, or nullptr when the port has none.
 */
iProgramLoaderInterface *getProgramLoader() {
    return s_program_loader;
}

#endif // ENABLE_PROGRAM_EXEC
