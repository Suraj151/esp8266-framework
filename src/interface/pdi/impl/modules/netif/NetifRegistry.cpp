/***************************** Netif Registry **********************************
This file is part of the PDI Stack.

This is free software. You can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
Created Date    : 27th Aug 2026
******************************************************************************/

#include <config/Config.h>

#ifdef ENABLE_NETWORK_SERVICE

#include "NetifRegistry.h"
#include <interface/pdi.h>

NetifRegistry __netif_registry;

NetifRegistry::NetifRegistry() : m_count(0) {
    for (uint8_t i = 0; i < NETIF_MAX_REGISTERED; i++) {
        m_netifs[i] = nullptr;
    }
}

/**
 * Take an interface into the table, refusing a duplicate name or a full
 * table rather than shadowing what is already there.
 */
int8_t NetifRegistry::registerNetif(iNetifInterface *netif) {
    if (nullptr == netif || nullptr == netif->name()) {
        return PDI_ERR_NULL_PTR;
    }

    if (m_count >= NETIF_MAX_REGISTERED) {
        return PDI_ERR_NO_SPACE;
    }

    if (nullptr != find(netif->name())) {
        return PDI_ERR_EXISTS;
    }

    m_netifs[m_count] = netif;
    m_count++;

    return (int8_t)(m_count - 1);
}

/**
 * Drop an interface, for a link that goes away while the device runs.
 */
bool NetifRegistry::unregisterNetif(iNetifInterface *netif) {
    if (nullptr == netif) {
        return false;
    }

    for (uint8_t i = 0; i < m_count; i++) {
        if (m_netifs[i] != netif) continue;

        for (uint8_t j = i; j + 1 < m_count; j++) {
            m_netifs[j] = m_netifs[j + 1];
        }

        m_count--;
        m_netifs[m_count] = nullptr;
        return true;
    }

    return false;
}

/**
 * The interface answering to a name, or nullptr when none does.
 */
iNetifInterface *NetifRegistry::find(const char *name) const {
    if (nullptr == name) {
        return nullptr;
    }

    for (uint8_t i = 0; i < m_count; i++) {
        if (nullptr == m_netifs[i]) continue;
        const char *have = m_netifs[i]->name();
        if (nullptr != have && 0 == strcmp(have, name)) {
            return m_netifs[i];
        }
    }

    return nullptr;
}

#endif
