/******************************* WiFi Netif ************************************
This file is part of the PDI Stack.

This is free software. You can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
Created Date    : 27th Aug 2026
******************************************************************************/

#include <config/Config.h>

#if defined(ENABLE_NETWORK_SERVICE) && defined(ENABLE_WIFI_SERVICE)

#include "WiFiNetif.h"
#include "NetifRegistry.h"
#include <interface/pdi.h>

WiFiStationNetif __netif_wlan0;
WiFiApNetif __netif_ap0;

/**
 * The station's addresses and the access point it is associated with.
 */
bool WiFiStationNetif::getInfo(netif_info_t &out) {
    out.m_kind = NETIF_KIND_WIFI_STA;
    out.m_up = __i_wifi.isConnected();
    out.m_ip = __i_wifi.localIP();
    out.m_netmask = __i_wifi.subnetMask();
    out.m_gateway = __i_wifi.gatewayIP();
    out.m_ssid = __i_wifi.SSID();
    out.m_rssi = __i_wifi.RSSI();

    pdiutil::string mac = __i_wifi.macAddress();
    uint32_t len = mac.length();
    if (len >= sizeof(out.m_mac)) len = sizeof(out.m_mac) - 1;
    memcpy(out.m_mac, mac.c_str(), len);
    out.m_mac[len] = '\0';

    return true;
}

/**
 * What the port can say about station traffic, and nothing when it cannot.
 */
bool WiFiStationNetif::getCounters(netif_counters_t &out) {
    return __i_wifi.getCounters(NETIF_KIND_WIFI_STA, out);
}

/**
 * The access point's own address and the network it advertises.
 */
bool WiFiApNetif::getInfo(netif_info_t &out) {
    out.m_kind = NETIF_KIND_WIFI_AP;
    out.m_ip = __i_wifi.softAPIP();
    out.m_netmask = __i_wifi.softAPSubnetMask();

    // there are no portable mode constants to ask, and an access point that
    // holds an address is one that came up
    out.m_up = out.m_ip.isSet();

    pdiutil::string mac = __i_wifi.softAPmacAddress();
    uint32_t len = mac.length();
    if (len >= sizeof(out.m_mac)) len = sizeof(out.m_mac) - 1;
    memcpy(out.m_mac, mac.c_str(), len);
    out.m_mac[len] = '\0';

    return true;
}

/**
 * What the port can say about access point traffic, and nothing when it
 * cannot.
 */
bool WiFiApNetif::getCounters(netif_counters_t &out) {
    return __i_wifi.getCounters(NETIF_KIND_WIFI_AP, out);
}

/**
 * Put both wifi interfaces in the registry. Safe to call more than once.
 */
void registerWiFiNetifs() {
    __netif_registry.registerNetif(&__netif_wlan0);
    __netif_registry.registerNetif(&__netif_ap0);
}

#endif
