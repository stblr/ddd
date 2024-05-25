#pragma once

#include "payload/network/UDPSocket.hh"

#include <common/Array.hh>
#include <common/Ring.hh>

class DNS {
public:
    bool resolve(const char *name, Array<u8, 4> &address);

    static void Init();
    static DNS *Instance();

private:
    struct Query {
        s64 expirationTime;
        u16 id;
        Array<char, 256> name;
    };

    struct Response {
        s64 expirationTime;
        Array<char, 256> name;
        Array<u8, 4> address;
    };

    DNS();

    bool readResponse(Response &response);
    bool writeQuery(const Query &query);

    alignas(0x20) UDPSocket m_socket;
    alignas(0x20) Array<u8, 512> m_buffer;
    Array<Socket::Address, 2> m_resolvers;
    u16 m_id;
    Ring<Query, 32> m_queries;
    Ring<Response, 256> m_responses;

    static DNS *s_instance;
};
