/**************************** Net Stack Interface ******************************
This file is part of the PDI Stack.

This is free software. You can redistribute it and/or modify it but without any
warranty.

The endpoints the network stack currently holds, as everything that reads them
sees it. A link type answers about its own interface through iNetifInterface;
this answers about the connections riding over all of them, which belong to the
stack rather than to any one interface.

A device layer implements this only if it can enumerate its stack. One that
cannot registers nothing, and the readers above leave the endpoint columns
empty rather than reporting a device with nothing listening.

Author          : Suraj I.
Created Date    : 29th Aug 2026
******************************************************************************/

#ifndef _I_NET_STACK_INTERFACE_H_
#define _I_NET_STACK_INTERFACE_H_

#include <interface/interface_includes.h>

/**
 * Receives one endpoint. Enumeration hands each socket over as it is found, so
 * a walk costs one record however many the stack holds.
 */
typedef void (*NetSocketVisitorFn)(const net_socket_t &sock, void *arg);

class iNetStackInterface {

public:
  /**
   * iNetStackInterface constructor.
   */
  iNetStackInterface() {}

  /**
   * iNetStackInterface destructor.
   */
  virtual ~iNetStackInterface() {}

  /**
   * Walk every TCP endpoint the stack holds, listening and connected alike,
   * passing each to the visitor. Returns false when the walk could not be made
   * at all, which is not the same as a stack holding nothing.
   */
  virtual bool eachTcpSocket(NetSocketVisitorFn visit, void *arg) = 0;
};

#endif
