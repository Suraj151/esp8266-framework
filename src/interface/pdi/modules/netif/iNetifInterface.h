/***************************** Netif Interface *********************************
This file is part of the PDI Stack.

This is free software. You can redistribute it and/or modify it but without any
warranty.

One network interface, as everything that reads the network sees it. A link
type implements this and registers itself; nothing above has to know whether
the bytes leave over wifi, ethernet or a modem.

Author          : Suraj I.
Created Date    : 27th Aug 2026
******************************************************************************/

#ifndef _I_NETIF_INTERFACE_H_
#define _I_NETIF_INTERFACE_H_

#include <interface/interface_includes.h>

class iNetifInterface {

public:
  /**
   * iNetifInterface constructor.
   */
  iNetifInterface() {}

  /**
   * iNetifInterface destructor.
   */
  virtual ~iNetifInterface() {}

  /**
   * The name this interface answers to, kept short enough to be a path
   * segment under /sys/class/net.
   */
  virtual const char *name() const = 0;

  /**
   * What the interface currently is: addresses, link state and whatever else
   * its kind can say about itself.
   */
  virtual bool getInfo(netif_info_t &out) = 0;

  /**
   * Traffic the interface has carried. An interface with no counters says so
   * rather than reporting zeroes that look like an idle link.
   */
  virtual bool getCounters(netif_counters_t &out) { return false; }
};

#endif
