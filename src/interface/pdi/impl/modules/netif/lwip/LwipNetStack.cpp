/******************************* lwIP Net Stack ********************************
This file is part of the PDI Stack.

This is free software. You can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
Created Date    : 29th Aug 2026
******************************************************************************/

#include "LwipNetStack.h"

#if defined(ENABLE_NETWORK_SERVICE) && defined(PDI_NET_STACK_LWIP)

extern "C" {
#include <lwip/tcpip.h>
#include <lwip/priv/tcp_priv.h>
}

LwipNetStack __lwip_net_stack;

namespace {

/**
 * Holds the tcpip core lock for a scope, and only when this thread is not
 * already the holder, since taking it twice would deadlock.
 */
struct LwipNetStackLockGuard {
    bool m_locked;

    LwipNetStackLockGuard() : m_locked(false) {
#if LWIP_TCPIP_CORE_LOCKING
        if (!sys_thread_tcpip(LWIP_CORE_LOCK_QUERY_HOLDER)) {
            LOCK_TCPIP_CORE();
            m_locked = true;
        }
#endif
    }

    ~LwipNetStackLockGuard() {
#if LWIP_TCPIP_CORE_LOCKING
        if (m_locked) UNLOCK_TCPIP_CORE();
#endif
    }
};

/**
 * A stack state as the framework names it, the two enums sharing TCP's own
 * ordering.
 */
net_sock_state_t toSockState(uint8_t state) {
    return (state < (uint8_t)NET_SOCK_MAX) ? (net_sock_state_t)state : NET_SOCK_CLOSED;
}

/**
 * An address as the framework holds one, the stored word already being in
 * network order.
 */
ipaddress_t toIpAddress(const ip_addr_t &addr) {
    const uint8_t *octets = (const uint8_t *)&(ip_2_ip4(&addr)->addr);
    return ipaddress_t(octets[0], octets[1], octets[2], octets[3]);
}

}

/**
 * Walk every TCP endpoint lwIP holds, listening, connected and timing out.
 */
bool LwipNetStack::eachTcpSocket(NetSocketVisitorFn visit, void *arg) {
    if (nullptr == visit) {
        return false;
    }

    LwipNetStackLockGuard guard;

    for (struct tcp_pcb_listen *lpcb = tcp_listen_pcbs.listen_pcbs;
         nullptr != lpcb; lpcb = lpcb->next) {

        if (!IP_IS_V4_VAL(lpcb->local_ip)) continue;

        net_socket_t sock;
        sock.m_localip = toIpAddress(lpcb->local_ip);
        sock.m_localport = lpcb->local_port;
        sock.m_state = NET_SOCK_LISTEN;

        visit(sock, arg);
    }

    struct tcp_pcb *const lists[2] = { tcp_active_pcbs, tcp_tw_pcbs };

    for (uint8_t i = 0; i < 2; i++) {
        for (struct tcp_pcb *pcb = lists[i]; nullptr != pcb; pcb = pcb->next) {

            if (!IP_IS_V4_VAL(pcb->local_ip)) continue;

            net_socket_t sock;
            sock.m_localip = toIpAddress(pcb->local_ip);
            sock.m_localport = pcb->local_port;
            sock.m_remoteip = toIpAddress(pcb->remote_ip);
            sock.m_remoteport = pcb->remote_port;
            sock.m_state = toSockState((uint8_t)pcb->state);

            visit(sock, arg);
        }
    }

    return true;
}

#endif // ENABLE_NETWORK_SERVICE && PDI_NET_STACK_LWIP
