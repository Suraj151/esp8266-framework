/**************************** WiFi Interface ***********************************
This file is part of the Ewings Esp Stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 1st June 2019
******************************************************************************/

#ifndef _WIFI_INTERFACE_H_
#define _WIFI_INTERFACE_H_

#include "iWiFiInterface.h"
#include <ESP8266WiFi.h>

/**
 * WiFiInterface class
 */
class WiFiInterface : public iWiFiInterface {

  public:

    /**
     * WiFiInterface constructor.
     */
    WiFiInterface();
    /**
     * WiFiInterface destructor.
     */
    ~WiFiInterface();

    // generic api's
    void setOutputPower(float _dBm);
    void persistent(bool _persistent);
    bool getPersistent();
    bool setMode(wifi_mode _mode);
    wifi_mode getMode();
    bool setSleepMode(sleep_mode type, uint8_t listenInterval = 0);
    sleep_mode getSleepMode();
    // bool setPhyMode(phy_mode mode);
    // phy_mode getPhyMode();
    bool enableSTA(bool _enable);
    bool enableAP(bool _enable);
    int hostByName(const char *aHostname, IPAddress &aResult);
    int hostByName(const char *aHostname, IPAddress &aResult, uint32_t timeout_ms);

    wifi_status begin(char *_ssid, char *_passphrase = nullptr, int32_t _channel = 0, const uint8_t *_bssid = nullptr, bool _connect = true);
    bool config(IPAddress &_local_ip, IPAddress &_gateway, IPAddress &_subnet);
    bool reconnect();
    bool disconnect(bool _wifioff = false);
    bool isConnected();
    bool setAutoConnect(bool _autoConnect);
    bool getAutoConnect();
    bool setAutoReconnect(bool _autoReconnect);
    bool getAutoReconnect();
    // int8_t waitForConnectResult(uint32_t _timeoutLength = 60000);

    // STA network info
    IPAddress localIP();
    uint8_t *macAddress(uint8_t *_mac);
    String macAddress();
    IPAddress subnetMask();
    IPAddress gatewayIP();
    IPAddress dnsIP(uint8_t _dns_no = 0);

    // STA WiFi info
    wifi_status status();
    String SSID() const;
    String psk() const;
    uint8_t *BSSID();
    String BSSIDstr();
    int32_t RSSI();

    // Soft AP api's
    bool softAP(const char *_ssid, const char *_passphrase = nullptr, int _channel = 1, int _ssid_hidden = 0, int _max_connection = 4);
    bool softAPConfig(IPAddress _local_ip, IPAddress _gateway, IPAddress _subnet);
    bool softAPdisconnect(bool _wifioff = false);
    uint8_t softAPgetStationNum();
    IPAddress softAPIP();
    uint8_t* softAPmacAddress(uint8_t *_mac);
    String softAPmacAddress(void);
    String softAPSSID() const;
    String softAPPSK() const;

    // n/w scan api's
    int8_t scanNetworks(bool _async = false, bool _show_hidden = false, uint8_t _channel = 0, uint8_t *ssid = nullptr);
    void scanNetworksAsync(std::function<void(int)> _onComplete, bool _show_hidden = false);
    // int8_t scanComplete();
    // void scanDelete();
    // uint8_t encryptionType(uint8_t _networkItem);
    // bool isHidden(uint8_t _networkItem);
    String SSID(uint8_t _networkItem);
    int32_t RSSI(uint8_t _networkItem);
    uint8_t * BSSID(uint8_t _networkItem);
    String BSSIDstr(uint8_t _networkItem);
    int32_t channel(uint8_t _networkItem);

    // misc api's
    static void preinitWiFiOff();

  private:

    /**
		 * @var	ESP8266WiFiClass*|&WiFi m_wifi
		 */
    ESP8266WiFiClass  *m_wifi;
};

extern WiFiInterface __wifi_interface;
#endif
