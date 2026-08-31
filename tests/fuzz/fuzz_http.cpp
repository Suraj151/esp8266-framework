/******************************* Fuzz HTTP ************************************
This file is part of the pdi stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

The request reader the portal sits behind: request line, headers, query string,
urlencoded form and the multipart body that carries a firmware image. All of it
runs before the session cookie is looked at.

Author          : Suraj I.
created Date    : 26th Aug 2026
******************************************************************************/

#include <FuzzCommon.h>

#ifdef ENABLE_HTTP_SERVER

#include <interface/pdi/impl/middlewares/HttpServerInterfaceImpl.h>
#ifdef ENABLE_STORAGE_SERVICE
#include <MountedStack.h>
#endif

namespace
{

    /**
     * @brief A server that hands out one client carrying the fuzz input.
     *
     * The reader refuses to run without a server, and it owns whatever it
     * accepts, so each round gets a fresh heap client.
     */
    class FuzzHttpListener : public iTcpServerInterface
    {

    public:
        FuzzHttpListener() : m_data(nullptr), m_size(0), m_pending(false) {}

        /**
         * @brief Queue one connection carrying these bytes.
         */
        void offer(const uint8_t *data, size_t size)
        {
            m_data = data;
            m_size = size;
            m_pending = true;
        }

        int32_t begin(uint16_t port) override { return 0; }

        bool hasClient() const override { return m_pending; }

        iClientInterface *accept() override
        {
            if (!m_pending)
            {
                return nullptr;
            }
            m_pending = false;
            return pdiutil::safe_new<pdifuzz::FuzzClient>(m_data, m_size);
        }

        void close() override { m_pending = false; }

    private:
        const uint8_t *m_data;
        size_t m_size;
        bool m_pending;
    };

    /**
     * @brief Reaches the server's own state, which it keeps internal.
     */
    class FuzzHttpServer : public HttpServerInterfaceImpl
    {

    public:
        /**
         * @brief Point the reader at the harness listener instead of a socket.
         */
        void listenOn(iTcpServerInterface *listener) { m_server = listener; }

        /**
         * @brief Serve one connection and leave nothing behind for the next.
         */
        void serveOne()
        {
            handleClient();

            closeClient();
            m_clientRequest.clear();
            purgeUploads();
        }

        /**
         * @brief Drop whatever a multipart body left in the temp directory.
         *
         * An upload is named by the request, so every round that carries one
         * writes another file. Left alone they accumulate in the emulated
         * flash and the run dies of memory exhaustion the framework never
         * caused.
         */
        void purgeUploads()
        {
#ifdef ENABLE_STORAGE_SERVICE
            const char *tempdir = __i_fs.getTempDirectory();
            if (nullptr == tempdir)
            {
                return;
            }

            pdiutil::vector<file_info_t> items;
            if (__i_fs.getDirFileList(tempdir, items) < 0)
            {
                return;
            }

            for (size_t i = 0; i < items.size(); i++)
            {
                if (nullptr != items[i].m_name)
                {
                    pdiutil::string path = pdiutil::string(tempdir);
                    __i_fs.appendFileSeparator(path);
                    path += items[i].m_name;
                    __i_fs.deleteFile(path.c_str());
                    pdiutil::safe_delete_array(items[i].m_name);
                }
            }
#endif
        }
    };

    FuzzHttpListener g_listener;
    FuzzHttpServer *g_server = nullptr;

    /**
     * @brief Bring the reader up once, with somewhere to route a request.
     */
    void readyServer()
    {
        if (nullptr != g_server)
        {
            return;
        }

#ifdef ENABLE_STORAGE_SERVICE
        pdifuzz::useFreeClock();
        pditest::mountedVfs();
#endif

        g_server = new FuzzHttpServer();
        g_server->listenOn(&g_listener);
        g_server->setStoragePath(FILE_SEPARATOR);
        g_server->on(pdiutil::string("/"), []() {});
        g_server->onNotFound([]() {});

        const char *headers[] = {"Cookie", "Content-Type", "Authorization"};
        g_server->collectHeaders(headers, 3);
    }

} // namespace

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    readyServer();

    g_listener.offer(data, size);
    g_server->serveOne();

    return 0;
}

#else

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    return 0;
}

#endif
