/*************************** Fuzz HTTP Response *******************************
This file is part of the pdi stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

The reply reader on the client side: the status line, the header split, the
chunked decoder and the body a download is handed chunk by chunk. Every byte of
it is chosen by whichever server the url named, and the url itself arrives from
a command line, so both are read from a stranger.

Author          : Suraj I.
created Date    : 11th Sep 2026
******************************************************************************/

#include <FuzzCommon.h>

#ifdef ENABLE_HTTP_CLIENT

#include <transports/http/HTTPClient.h>

namespace
{

    /**
     * @brief A client that reads a reply the way a download does.
     *
     * The reader is protected and the body only streams with a writer
     * installed, so both are reached through a subclass rather than through a
     * request that would need a server to answer it.
     */
    class FuzzHttpResponse : public Http_Client
    {

    public:
        /**
         * @brief Read one reply, streaming the body when a writer is given.
         */
        int16_t readReply(iClientInterface *client, CallBackBytesArgBoolRetFn writer)
        {
            m_client = client;
            m_stream_writer = writer;
            m_stream_bytes_written = 0;
            return handleResponse();
        }
    };

} // namespace

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    pdifuzz::useFreeClock();

    pdifuzz::FuzzInput input(data, size);
    uint8_t mode = input.pick(3);
    uint8_t stop_after = input.pick(8);

    if (2 == mode)
    {
        pdiutil::string url((const char *)input.rest(), input.remaining());

        FuzzHttpResponse reader;
        reader.SetUrl(url.c_str());
        return 0;
    }

    // Http_Client nulls its client pointer on End() and never takes ownership,
    // so this one is a stack object where the session readers need a heap one
    pdifuzz::FuzzClient client(input.rest(), input.remaining());

    uint32_t calls = 0;
    CallBackBytesArgBoolRetFn writer = [&](const uint8_t *buf, uint32_t sz) -> bool
    {
        calls++;
        return (0 == stop_after) || (calls <= stop_after);
    };

    FuzzHttpResponse reader;
    reader.readReply(&client, (1 == mode) ? writer : nullptr);

    char *value = nullptr;
    reader.GetRespHeader("content-length", value);

    return 0;
}

#else

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    return 0;
}

#endif
