/**************************** Net Stack Interface ******************************
This file is part of the pdi stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 29th Aug 2026
******************************************************************************/

#ifndef _PDI_POSIX_NET_STACK_INTERFACE_H_
#define _PDI_POSIX_NET_STACK_INTERFACE_H_

#include "posix.h"
#include <interface/interface_includes.h>

#ifdef ENABLE_NETWORK_SERVICE

#include <interface/pdi/modules/netif/iNetStackInterface.h>

/**
 * NetStackInterface class
 */
class NetStackInterface : public iNetStackInterface
{

public:
  /**
   * NetStackInterface constructor.
   */
  NetStackInterface() {}

  /**
   * NetStackInterface destructor.
   */
  ~NetStackInterface() {}

  /**
   * Walk every TCP socket the process holds, listening and connected alike.
   */
  bool eachTcpSocket(NetSocketVisitorFn visit, void *arg) override;
};

extern NetStackInterface __i_net_stack;

#endif // ENABLE_NETWORK_SERVICE

#endif // _PDI_POSIX_NET_STACK_INTERFACE_H_
