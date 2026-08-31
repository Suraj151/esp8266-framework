/***************************** Netif Registry **********************************
This file is part of the PDI Stack.

This is free software. You can redistribute it and/or modify it but without any
warranty.

The table of network interfaces the device has brought up, in the order they
registered. A link type registers once at start up and everything that reads
the network - /sys/class/net, /proc/net, the commands - enumerates from here.

Author          : Suraj I.
Created Date    : 27th Aug 2026
******************************************************************************/

#ifndef _NETIF_REGISTRY_H_
#define _NETIF_REGISTRY_H_

#include <config/Config.h>

#ifdef ENABLE_NETWORK_SERVICE

#include <interface/pdi/modules/netif/iNetifInterface.h>
#include <interface/pdi/modules/netif/iNetStackInterface.h>
#include <config/NetworkConfig.h>

class NetifRegistry {

public:
  NetifRegistry();
  ~NetifRegistry() {}

  /**
   * Take an interface into the table, refusing a duplicate name or a full
   * table rather than shadowing what is already there.
   */
  int8_t registerNetif(iNetifInterface *netif);

  /**
   * Drop an interface, for a link that goes away while the device runs.
   */
  bool unregisterNetif(iNetifInterface *netif);

  /**
   * How many interfaces are registered.
   */
  uint8_t count() const { return m_count; }

  /**
   * The interface at an index, or nullptr past the end.
   */
  iNetifInterface *at(uint8_t idx) const {
    return (idx < m_count) ? m_netifs[idx] : nullptr;
  }

  /**
   * The interface answering to a name, or nullptr when none does.
   */
  iNetifInterface *find(const char *name) const;

  /**
   * Take the stack these interfaces carry, for readers that ask about
   * endpoints rather than links.
   */
  void registerStack(iNetStackInterface *stack) { m_stack = stack; }

  /**
   * The registered stack, or nullptr when the port cannot enumerate one.
   */
  iNetStackInterface *stack() const { return m_stack; }

private:
  iNetifInterface *m_netifs[NETIF_MAX_REGISTERED];
  iNetStackInterface *m_stack;
  uint8_t m_count;
};

extern NetifRegistry __netif_registry;

#endif // ENABLE_NETWORK_SERVICE

#endif
