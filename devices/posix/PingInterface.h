/****************************** Ping Interface *******************************
This file is part of the pdi stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 1st June 2019
******************************************************************************/

#ifndef _PDI_POSIX_PING_INTERFACE_H_
#define _PDI_POSIX_PING_INTERFACE_H_

#include "posix.h"
#include <interface/pdi/middlewares/iPingInterface.h>

#ifndef PDI_POSIX_PING_MAX_HOSTS
#define PDI_POSIX_PING_MAX_HOSTS 4
#endif

#ifndef PDI_POSIX_PING_DEFAULT_RTT_MS
#define PDI_POSIX_PING_DEFAULT_RTT_MS 8
#endif

#ifndef PDI_POSIX_PING_DEFAULT_INTERVAL_MS
#define PDI_POSIX_PING_DEFAULT_INTERVAL_MS 10
#endif

#ifndef PDI_POSIX_PING_DEFAULT_TIMEOUT_MS
#define PDI_POSIX_PING_DEFAULT_TIMEOUT_MS 40
#endif

#ifdef MOCK_DEVICE_TEST
/**
 * how one scripted host answers an echo request
 */
struct mock_ping_host_t
{
    uint32_t m_ip;
    bool m_reachable;
    uint32_t m_rtt_ms;

    mock_ping_host_t();
};
#endif

/**
 * @class PingInterface
 * @brief Echo requests over icmp, or a scripted stand in for them under test.
 *
 * A real build sends icmp echo requests from an unprivileged datagram socket,
 * falling back to a raw one, and reads the replies back. Under
 * MOCK_DEVICE_TEST nothing leaves the process: a caller says which addresses
 * answer and how quickly, so a run is repeatable and needs no network.
 *
 * Either way packets are reported one at a time from service(), which the
 * device control interface calls out of yield and handleEvents, so a caller
 * waiting on isPingComplete sees the ordering it sees on hardware.
 */
class PingInterface : public iPingInterface
{

public:
    PingInterface();
    ~PingInterface();

    void init_ping(iWiFiInterface *_wifi) override;
    bool ping(const ipaddress_t &target, uint16_t count = 1,
              CallBackVoidPointerArgFn on_packet = nullptr) override;
    bool isPingComplete(void) override;
    bool isHostRespondingToPing(void) override;
    const ping_stats_t &getPingStats(void) override;
    bool isPingBusy(void) override;

#ifdef MOCK_DEVICE_TEST
    /**
     * @brief Decide how one address answers. Overrides the default policy.
     */
    void setHostReachable(const ipaddress_t &target, bool reachable,
                          uint32_t rtt_ms = PDI_POSIX_PING_DEFAULT_RTT_MS);

    /**
     * @brief Answer for every address no host entry covers.
     */
    void setDefaultReachable(bool reachable, uint32_t rtt_ms = PDI_POSIX_PING_DEFAULT_RTT_MS);

    /**
     * @brief Change each successive reply's round trip by this much, so a run
     *        produces a spread of times rather than one repeated value.
     */
    void setRttStep(int32_t step_ms);

    /**
     * @brief Lose the packet with this sequence number even from a reachable
     *        host. Sequence numbers are 1 based and up to 32 are honoured.
     */
    void dropPacket(uint16_t seqno);

    /**
     * @brief Gap between one packet's outcome and the next request.
     */
    void setPacketInterval(uint32_t interval_ms);

    /**
     * @brief How long a lost packet takes to be declared lost.
     */
    void setTimeout(uint32_t timeout_ms);

    /**
     * @brief Forget every scripted host, the drop list and the timings.
     */
    void clearScript();
#endif

    /**
     * @brief Report whichever packets have come due.
     * @return number of packets reported.
     */
    uint16_t service();

protected:
    /**
     * @var iWiFiInterface* wifi
     */
    iWiFiInterface *m_wifi;

#ifdef MOCK_DEVICE_TEST
    mock_ping_host_t m_hosts[PDI_POSIX_PING_MAX_HOSTS];
    bool m_default_reachable;
    uint32_t m_default_rtt_ms;
    int32_t m_rtt_step_ms;
    uint32_t m_drop_mask;
    bool m_target_reachable;
    uint32_t m_target_rtt_ms;
#else
    /**
     * @brief Opens the echo socket, preferring the unprivileged datagram form.
     */
    bool openSocket();

    /**
     * @brief Sends one echo request and notes when it went out.
     */
    bool sendEcho();

    /**
     * @brief Takes a reply if one is waiting, reporting its round trip.
     */
    bool takeReply(uint32_t &rtt_ms);

    /**
     * @brief Records one packet's outcome and hands it to the caller.
     */
    void reportPacket(bool replied, uint32_t rtt_ms);

    int32_t m_sock;
    bool m_raw;
    uint16_t m_id;
    uint32_t m_dest_ip;
    uint32_t m_sent_at_ms;
    bool m_outstanding;
#endif

    uint32_t m_interval_ms;
    uint32_t m_timeout_ms;

    ping_stats_t m_stats;
    uint32_t m_sum_ms;
    uint16_t m_seq;
    uint16_t m_count;
    bool m_running;
    bool m_complete;
    bool m_host_resp;
    CallBackVoidPointerArgFn m_pkt_cb;
    uint32_t m_next_due_ms;
};

#endif
