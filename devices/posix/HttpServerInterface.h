/************************* Http Server Interface ******************************
This file is part of the pdi stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 16th Aug 2026
******************************************************************************/

#ifndef _PDI_POSIX_HTTP_SERVER_INTERFACE_H_
#define _PDI_POSIX_HTTP_SERVER_INTERFACE_H_

#include "posix.h"
#include <interface/pdi/impl/middlewares/HttpServerInterfaceImpl.h>

/**
 * HttpServerInterface class
 */
class HttpServerInterface : public HttpServerInterfaceImpl
{

public:
  /**
   * HttpServerInterface constructor.
   */
  HttpServerInterface();

  /**
   * HttpServerInterface destructor.
   */
  ~HttpServerInterface();
};

#endif // _PDI_POSIX_HTTP_SERVER_INTERFACE_H_
