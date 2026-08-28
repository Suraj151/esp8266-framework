/******************************* Fuzz Common **********************************
This file is part of the pdi stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Shared pieces for the fuzz harnesses: a cursor that splits one input into the
fields a target needs, and a client backed by that input so a parser reading
from the network can be driven without a socket.

Author          : Suraj I.
created Date    : 26th Aug 2026
******************************************************************************/

#ifndef _PDI_FUZZ_COMMON_H_
#define _PDI_FUZZ_COMMON_H_

#include <interface/pdi.h>
#include <interface/pdi/middlewares/iClientInterface.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

namespace pdifuzz
{

    /**
     * @brief Make waiting free, so a read timeout costs no wall clock.
     *
     * The parsers under test wait on peers that are never coming. Left on the
     * real clock a single input can burn its whole read budget in seconds; on
     * the virtual clock the same deadline still expires, just at once.
     */
    inline void useFreeClock()
    {
        static bool done = false;
        if (!done)
        {
            done = true;
            __i_dvc_ctrl.useVirtualClock(true);
        }
    }

    /**
     * @brief A cursor over one fuzz input, handing out selectors and slices.
     *
     * Targets that need more than one field take them from the front, so the
     * mutator can steer a run by changing the first byte alone.
     */
    class FuzzInput
    {

    public:
        FuzzInput(const uint8_t *data, size_t size) : m_data(data), m_size(size), m_pos(0) {}

        /**
         * @brief One byte reduced to the given range, or zero once drained.
         */
        uint8_t pick(uint8_t choices)
        {
            if (m_pos >= m_size || 0 == choices)
            {
                return 0;
            }
            return (uint8_t)(m_data[m_pos++] % choices);
        }

        /**
         * @brief Everything not yet handed out.
         */
        const uint8_t *rest() const { return m_data + m_pos; }

        /**
         * @brief How many bytes rest() points at.
         */
        size_t remaining() const { return m_size - m_pos; }

        /**
         * @brief The remainder as the byte vector the ssh parsers take.
         */
        pdiutil::vector<uint8_t> restAsVector() const
        {
            return pdiutil::vector<uint8_t>(rest(), rest() + remaining());
        }

    private:
        const uint8_t *m_data;
        size_t m_size;
        size_t m_pos;
    };

    /**
     * @brief A tcp client whose received bytes are the fuzz input.
     *
     * It models a peer that sent exactly these bytes and then hung up: writes
     * are counted and dropped, and the connection reads as closed once the
     * input is drained, so a reader waiting for more leaves by the path a
     * departed peer would take rather than sitting on a deadline.
     */
    class FuzzClient : public iTcpClientInterface
    {

    public:
        FuzzClient(const uint8_t *data, size_t size) : m_data(data),
                                                       m_size(size),
                                                       m_pos(0),
                                                       m_written(0),
                                                       m_connected(true) {}
        ~FuzzClient() {}

        using iTcpClientInterface::read;
        using iTcpClientInterface::write;

        /**
         * @brief Bytes the unit under test has written back.
         */
        uint32_t written() const { return m_written; }

        int16_t disconnect() override
        {
            m_connected = false;
            return 0;
        }

        int32_t write(uint8_t c) override
        {
            m_written++;
            return 1;
        }

        int32_t write(const uint8_t *c_str) override
        {
            if (nullptr == c_str)
            {
                return 0;
            }
            return write(c_str, (uint32_t)strlen((const char *)c_str));
        }

        int32_t write(const uint8_t *c_str, uint32_t size) override
        {
            if (nullptr == c_str || 0 == size)
            {
                return 0;
            }
            m_written += size;
            return (int32_t)size;
        }

        int32_t write_ro(const char *c_str) override
        {
            return write((const uint8_t *)c_str);
        }

        uint8_t read() override
        {
            if (m_pos >= m_size)
            {
                return 0;
            }
            return m_data[m_pos++];
        }

        int32_t read(uint8_t *buf, uint32_t size) override
        {
            uint32_t count = 0;
            while (count < size && m_pos < m_size)
            {
                buf[count++] = m_data[m_pos++];
            }
            return (int32_t)count;
        }

        int32_t available() override { return (int32_t)(m_size - m_pos); }

        int8_t connected() override { return (m_connected && m_pos < m_size) ? 1 : 0; }

        void setTimeout(uint32_t timeout) override {}

        ipaddress_t getLocalIp() const override { return ipaddress_t(); }
        uint16_t getLocalPort() const override { return 0; }
        ipaddress_t getRemoteIp() const override { return ipaddress_t(); }
        uint16_t getRemotePort() const override { return 0; }
        bool setKeepAlive(uint16_t idleTime, uint16_t interval, uint16_t count) override { return true; }
        void setNoDelay(bool noDelay) override {}

    private:
        const uint8_t *m_data;
        size_t m_size;
        size_t m_pos;
        uint32_t m_written;
        bool m_connected;
    };

} // namespace pdifuzz

#endif // _PDI_FUZZ_COMMON_H_
