/************************** Telnet service ***********************************
This file is part of the pdi stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 1st May 2025
******************************************************************************/

#ifndef _TELNET_SERVICE_PROVIDER_H_
#define _TELNET_SERVICE_PROVIDER_H_

#include <service_provider/ServiceProvider.h>

/**
 * @brief TelnetServiceProvider provides a basic Telnet server using TcpServerInterface.
 */
class TelnetServiceProvider : public ServiceProvider {
public:
    TelnetServiceProvider();
    virtual ~TelnetServiceProvider();

    /**
     * @brief Start the Telnet service on the specified port.
     * @param port The port to listen on (default 23).
     * @return true if started successfully, false otherwise.
     */
    bool start(uint16_t port = TELNET_DEFAULT_PORT);

    /**
     * @brief Init the service.
     * @param arg Optional argument, can be used to pass the port number.
     * @return true if initialized successfully, false otherwise.
     */
    bool initService(void *arg = nullptr) override;

    /**
     * Needs the network it listens on before it can come up.
     */
    uint8_t getServiceDependencies(service_t *_out, uint8_t _max) const override {
      uint8_t _n = 0;
    #ifdef ENABLE_WIFI_SERVICE
      if( _max > _n ) _out[_n++] = SERVICE_WIFI;
    #endif
      return _n;
    }

    /**
     * @brief Stop the Telnet service.
     */
    void stop();

    /**
     * Releases the listener and every pooled client, so a stopped telnet service
     * refuses a connection rather than leaving it to hang without a banner.
     */
    bool stopService() override;

    /**
     * Clear the pool-full grace window, so a restart does not open with a
     * period that appears to have been full since the previous run.
     */
    void resetServiceState() override;

    terminal_types_t getServiceTerminalType() const override { return TERMINAL_TYPE_TELNET; }

    /**
     * @brief close the client held in one pool slot.
     * @param slot Pool slot to close.
     */
    void closeClient(uint8_t slot);

    /**
     * @brief close every client the pool holds.
     */
    void closeAllClients();

    /**
     * @brief Handle incoming Telnet clients and data.
     */
    void handle();

private:
    /**
     * @brief Run the idle check and the input pass for one pool slot.
     * @param slot Pool slot to service.
     */
    void serviceClient(uint8_t slot);

    iTcpServerInterface* m_server;
    iClientInterface* m_clients[TELNET_MAX_SESSIONS];
    uint64_t m_last_activity[TELNET_MAX_SESSIONS];
    uint32_t m_poolfullsince;
};

extern TelnetServiceProvider __telnet_service;

#endif