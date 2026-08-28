/******************************* WiFi Netif ************************************
This file is part of the PDI Stack.

This is free software. You can redistribute it and/or modify it but without any
warranty.

Presents the wifi radio as the two interfaces it really is, a station and a
soft access point, over the wifi port every board already implements. A board
that grows a second link type registers its own alongside these rather than
changing anything here.

Author          : Suraj I.
Created Date    : 27th Aug 2026
******************************************************************************/

#ifndef _WIFI_NETIF_H_
#define _WIFI_NETIF_H_

#include <config/Config.h>

#if defined(ENABLE_NETWORK_SERVICE) && defined(ENABLE_WIFI_SERVICE)

#include <interface/pdi/modules/netif/iNetifInterface.h>

class WiFiStationNetif : public iNetifInterface {

public:
  WiFiStationNetif() {}
  ~WiFiStationNetif() {}

  const char *name() const override { return "wlan0"; }

  /**
   * The station's addresses and the access point it is associated with.
   */
  bool getInfo(netif_info_t &out) override;
};

class WiFiApNetif : public iNetifInterface {

public:
  WiFiApNetif() {}
  ~WiFiApNetif() {}

  const char *name() const override { return "ap0"; }

  /**
   * The access point's own address and the network it advertises.
   */
  bool getInfo(netif_info_t &out) override;
};

/**
 * Put both wifi interfaces in the registry. Safe to call more than once.
 */
void registerWiFiNetifs();

extern WiFiStationNetif __netif_wlan0;
extern WiFiApNetif __netif_ap0;

#endif // ENABLE_NETWORK_SERVICE && ENABLE_WIFI_SERVICE

#endif
