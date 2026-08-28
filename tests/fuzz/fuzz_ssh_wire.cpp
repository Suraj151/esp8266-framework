/***************************** Fuzz SSH Wire **********************************
This file is part of the pdi stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

The ssh parsers that run before any credential is checked. A client reaches
every one of these by opening a socket, so a length field that is trusted here
is trusted from an unauthenticated peer.

Author          : Suraj I.
created Date    : 26th Aug 2026
******************************************************************************/

#include <FuzzCommon.h>

#ifdef ENABLE_SSH_SERVICE

#include <service_provider/shell/ssh/SSHServiceUtil.h>

using namespace LWSSH;

namespace
{

    /**
     * @brief Drive the length-prefixed field readers over one payload.
     */
    void fuzzFieldReaders(const pdiutil::vector<uint8_t> &payload)
    {
        int32_t offset = 0;
        pdiutil::string text;
        read_ssh_string(payload, text, offset);

        offset = 0;
        pdiutil::vector<uint8_t> blob;
        read_ssh_string(payload, blob, offset);

        uint32_t nameoffset = 0;
        ssh_name_list names;
        parse_name_list(payload, nameoffset, names);

        pdiutil::vector<uint8_t> field;
        extract_ed25519_blob_field(payload, field, 32);
    }

    /**
     * @brief Read one packet off the wire the way the session loop does.
     */
    void fuzzWirePacket(const uint8_t *data, size_t size)
    {
        // the session takes ownership of the client and closes it on the way
        // out, so it cannot be handed one that lives on the stack
        pdifuzz::FuzzClient *client = pdiutil::safe_new<pdifuzz::FuzzClient>(data, size);
        if (nullptr == client)
        {
            return;
        }

        LWSSHSession session(client);
        ssh_packet packet;
        parse_received_packet(&session, packet);
    }

} // namespace

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    pdifuzz::FuzzInput input(data, size);
    uint8_t target = input.pick(7);
    pdiutil::vector<uint8_t> payload = input.restAsVector();

    switch (target)
    {
    case 0:
    {
        SSHKexInitFields fields;
        parse_kex_init_fields(payload, fields);
        break;
    }
    case 1:
    {
        EcdhInitPacket packet;
        parse_kex_ecdh_init(payload, packet);
        break;
    }
    case 2:
    {
        SSHUserAuthRequest request;
        parse_userauth_request(payload, request);
        break;
    }
    case 3:
    {
        SSHChannelRequest request;
        parse_channel_request(payload, request);
        break;
    }
    case 4:
    {
        SSHChannelData channeldata;
        parse_channel_data_request(payload, channeldata);
        break;
    }
    case 5:
        fuzzFieldReaders(payload);
        break;
    default:
        fuzzWirePacket(input.rest(), input.remaining());
        break;
    }

    return 0;
}

#else

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    return 0;
}

#endif
