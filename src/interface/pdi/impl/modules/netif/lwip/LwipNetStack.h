/******************************* lwIP Net Stack ********************************
This file is part of the PDI Stack.

This is free software. You can redistribute it and/or modify it but without any
warranty.

The endpoint walk shared by every port whose network runs on lwIP, reporting
the stack's own pcb lists. A port declares PDI_NET_STACK_LWIP to get it and
registers the instance from its own init.

Author          : Suraj I.
Created Date    : 29th Aug 2026
******************************************************************************/

#ifndef _LWIP_NET_STACK_H_
#define _LWIP_NET_STACK_H_

#include <config/Config.h>

#if defined(ENABLE_NETWORK_SERVICE) && defined(PDI_NET_STACK_LWIP)

#include <interface/pdi/modules/netif/iNetStackInterface.h>

class LwipNetStack : public iNetStackInterface {

public:
  LwipNetStack() {}
  ~LwipNetStack() {}

  /**
   * Walk every TCP endpoint lwIP holds, listening, connected and timing out.
   */
  bool eachTcpSocket(NetSocketVisitorFn visit, void *arg) override;
};

extern LwipNetStack __lwip_net_stack;

#endif // ENABLE_NETWORK_SERVICE && PDI_NET_STACK_LWIP

#endif
