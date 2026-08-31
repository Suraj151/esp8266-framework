/**************************** Net Stack Interface ******************************
This file is part of the pdi stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 29th Aug 2026
******************************************************************************/

#include "NetStackInterface.h"

#ifdef ENABLE_NETWORK_SERVICE

#include <sys/socket.h>
#include <sys/resource.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

NetStackInterface __i_net_stack;

namespace {

const int32_t NET_STACK_MAX_FD_SCAN = 256;

/**
 * Fill one endpoint of a socket, and say whether it had an address at all.
 */
bool readEndpoint(int32_t fd, bool remote, ipaddress_t &ip_out, uint16_t &port_out) {
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);

    memset(&addr, 0, sizeof(addr));

    int32_t res = remote ? getpeername(fd, (struct sockaddr *)&addr, &len)
                         : getsockname(fd, (struct sockaddr *)&addr, &len);

    if (0 != res || AF_INET != addr.sin_family) {
        return false;
    }

    // s_addr is already in network order, so its bytes are the octets in the
    // order they are written
    const uint8_t *octets = (const uint8_t *)&addr.sin_addr.s_addr;
    ip_out = ipaddress_t(octets[0], octets[1], octets[2], octets[3]);
    port_out = (uint16_t)ntohs(addr.sin_port);

    return true;
}

/**
 * What state the host says the connection is in, falling back to what having a
 * peer implies.
 */
net_sock_state_t readState(int32_t fd, bool haspeer) {
#if defined(__linux__) && defined(TCP_INFO)
    struct tcp_info info;
    socklen_t len = sizeof(info);

    memset(&info, 0, sizeof(info));

    if (0 == getsockopt(fd, IPPROTO_TCP, TCP_INFO, &info, &len)) {
        switch (info.tcpi_state) {
            case TCP_ESTABLISHED: return NET_SOCK_ESTABLISHED;
            case TCP_SYN_SENT:    return NET_SOCK_SYN_SENT;
            case TCP_SYN_RECV:    return NET_SOCK_SYN_RCVD;
            case TCP_FIN_WAIT1:   return NET_SOCK_FIN_WAIT_1;
            case TCP_FIN_WAIT2:   return NET_SOCK_FIN_WAIT_2;
            case TCP_TIME_WAIT:   return NET_SOCK_TIME_WAIT;
            case TCP_CLOSE_WAIT:  return NET_SOCK_CLOSE_WAIT;
            case TCP_LAST_ACK:    return NET_SOCK_LAST_ACK;
            case TCP_CLOSING:     return NET_SOCK_CLOSING;
            case TCP_LISTEN:      return NET_SOCK_LISTEN;
            default:              return NET_SOCK_CLOSED;
        }
    }
#endif

    return haspeer ? NET_SOCK_ESTABLISHED : NET_SOCK_CLOSED;
}

}

/**
 * Walk every TCP socket the process holds, listening and connected alike.
 */
bool NetStackInterface::eachTcpSocket(NetSocketVisitorFn visit, void *arg) {
    if (nullptr == visit) {
        return false;
    }

    int32_t limit = NET_STACK_MAX_FD_SCAN;
    struct rlimit lim;

    if (0 == getrlimit(RLIMIT_NOFILE, &lim) && (int32_t)lim.rlim_cur < limit) {
        limit = (int32_t)lim.rlim_cur;
    }

    for (int32_t fd = 0; fd < limit; fd++) {

        int32_t type = 0;
        socklen_t len = sizeof(type);

        if (0 != getsockopt(fd, SOL_SOCKET, SO_TYPE, &type, &len) || SOCK_STREAM != type) {
            continue;
        }

        net_socket_t sock;

        if (!readEndpoint(fd, false, sock.m_localip, sock.m_localport)) {
            continue;
        }

        bool haspeer = readEndpoint(fd, true, sock.m_remoteip, sock.m_remoteport);

        int32_t listening = 0;
        len = sizeof(listening);

        if (0 == getsockopt(fd, SOL_SOCKET, SO_ACCEPTCONN, &listening, &len) && 0 != listening) {
            sock.m_state = NET_SOCK_LISTEN;
        } else {
            sock.m_state = readState(fd, haspeer);
        }

        visit(sock, arg);
    }

    return true;
}

#endif // ENABLE_NETWORK_SERVICE
