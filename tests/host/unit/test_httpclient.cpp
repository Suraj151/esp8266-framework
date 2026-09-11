/**************************** HTTP Client Tests *******************************
This file is part of the pdi stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Covers how a response is read back: which line the status is taken from, and
that a header carrying a version string of its own cannot stand in for it.

Author          : Suraj I.
created Date    : 10th Sep 2026
******************************************************************************/

#include <MountedStack.h>
#include <pditest.h>

#include <transports/http/HTTPClient.h>

#ifdef ENABLE_HTTP_CLIENT

namespace {

/**
 * A client that answers from a scripted response instead of a socket, so a
 * reply can be shaped byte for byte and no listener is needed.
 */
class ReplayClient : public iClientInterface
{

public:

    ReplayClient(const char *response) : m_at(0), m_connected(1)
    {
        m_response = response;
        m_size = (uint32_t)strlen(response);
    }

    int16_t connect(const uint8_t *host, uint16_t port) override { m_connected = 1; return 1; }
    int16_t disconnect() override { m_connected = 0; return 1; }
    int8_t connected() override { return m_connected; }
    void setTimeout(uint32_t timeout) override {}

    int32_t write(uint8_t c) override { return 1; }
    int32_t write(const uint8_t *c_str) override { return (int32_t)strlen((const char *)c_str); }
    int32_t write(const uint8_t *c_str, uint32_t size) override { return (int32_t)size; }

    int32_t available() override { return (int32_t)(m_size - m_at); }

    uint8_t read() override
    {
        if (m_at >= m_size) return 0;
        return (uint8_t)m_response[m_at++];
    }

    int32_t read(uint8_t *buf, uint32_t size) override
    {
        uint32_t taken = 0;
        while (taken < size && m_at < m_size) {
            buf[taken++] = (uint8_t)m_response[m_at++];
        }
        return (int32_t)taken;
    }

private:

    const char *m_response;
    uint32_t m_size;
    uint32_t m_at;
    int8_t m_connected;
};

/**
 * Runs one scripted response through a download and reports what the body
 * collector received, so a status misread shows up as a refused transfer.
 */
static int64_t downloadThrough(const char *response, pdiutil::string &body,
                               const char *from = nullptr)
{
    ReplayClient client(response);
    Http_Client http;

    http.Begin();
    http.SetClient(&client);

    body.clear();

    pdiutil::string url = CHARPTR_WRAP("http://127.0.0.1:8080/f.txt");
    if (nullptr != from)
    {
        url = from;
    }
    int64_t got = http.DownloadStream(url.c_str(), [&body](const uint8_t *buf, uint32_t sz) -> bool {
        if (nullptr != buf) {
            body.append((const char *)buf, (pdiutil::string::size_type)sz);
        }
        return true;
    });

    http.End();

    return got;
}

}

TEST(httpclient, a_plain_response_streams_its_body)
{
    pdiutil::string body;
    int64_t got = downloadThrough(
        "HTTP/1.1 200 OK\r\nContent-Length: 5\r\n\r\nhello", body);

    ASSERT_EQ((int)got, 5);
    ASSERT_STREQ(body.c_str(), "hello");
}

TEST(httpclient, a_server_header_naming_a_version_does_not_become_the_status)
{
    pdiutil::string body;
    int64_t got = downloadThrough(
        "HTTP/1.1 200 OK\r\n"
        "Server: SimpleHTTP/0.6 Python/3.12.3\r\n"
        "Content-Length: 5\r\n\r\nhello", body);

    ASSERT_EQ((int)got, 5);
    ASSERT_STREQ(body.c_str(), "hello");
}

TEST(httpclient, a_header_carrying_a_whole_status_line_does_not_replace_it)
{
    pdiutil::string body;
    int64_t got = downloadThrough(
        "HTTP/1.1 200 OK\r\n"
        "X-Note: HTTP/9.9 404 nope\r\n"
        "Content-Length: 5\r\n\r\nhello", body);

    ASSERT_EQ((int)got, 5);
    ASSERT_STREQ(body.c_str(), "hello");
}

TEST(httpclient, a_status_that_is_not_ok_refuses_the_transfer)
{
    pdiutil::string body;
    int64_t got = downloadThrough(
        "HTTP/1.1 404 Not Found\r\nContent-Length: 3\r\n\r\nno!", body);

    ASSERT_LT((int)got, 0);
    ASSERT_TRUE(body.empty());
}

TEST(httpclient, a_status_on_the_first_line_is_read_whatever_the_version)
{
    pdiutil::string body;
    int64_t got = downloadThrough(
        "HTTP/1.0 200 OK\r\nContent-Length: 5\r\n\r\nhello", body);

    ASSERT_EQ((int)got, 5);
    ASSERT_STREQ(body.c_str(), "hello");
}

TEST(httpclient, a_reply_whose_lines_end_in_a_bare_feed_is_read)
{
    pdiutil::string body;
    int64_t got = downloadThrough(
        "HTTP/1.1 200 OK\nContent-Length: 5\n\nhello", body);

    ASSERT_EQ((int)got, 5);
    ASSERT_STREQ(body.c_str(), "hello");
}

TEST(httpclient, a_reply_whose_header_block_is_empty_is_read)
{
    pdiutil::string body;
    int64_t got = downloadThrough("HTTP/1.1 200 OK\n\nhello", body);

    ASSERT_EQ((int)got, 5);
    ASSERT_STREQ(body.c_str(), "hello");
}

TEST(httpclient, a_url_carrying_a_colon_without_a_host_is_refused)
{
    pdiutil::string body;
    const char *reply = "HTTP/1.1 200 OK\r\nContent-Length: 5\r\n\r\nhello";

    ASSERT_TRUE(downloadThrough(reply, body, "http:") < 0);
    ASSERT_TRUE(downloadThrough(reply, body, "https:") < 0);
    ASSERT_TRUE(downloadThrough(reply, body, "http:/") < 0);
    ASSERT_TRUE(downloadThrough(reply, body, "nohost:x") < 0);
}

TEST(httpclient, a_url_padded_with_leading_space_is_still_read)
{
    pdiutil::string body;
    int64_t got = downloadThrough(
        "HTTP/1.1 200 OK\r\nContent-Length: 5\r\n\r\nhello", body,
        "   http://127.0.0.1:8080/f.txt");

    ASSERT_EQ((int)got, 5);
    ASSERT_STREQ(body.c_str(), "hello");
}

TEST(httpclient, a_path_naming_https_does_not_make_a_plain_url_secure)
{
    pdiutil::string body;
    int64_t got = downloadThrough(
        "HTTP/1.1 200 OK\r\nContent-Length: 5\r\n\r\nhello", body,
        "http://127.0.0.1:8080/redirect?to=https");

    ASSERT_EQ((int)got, 5);
    ASSERT_STREQ(body.c_str(), "hello");
}

#endif
