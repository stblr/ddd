#pragma once

#include "payload/crypto/DH.hh"
#include "payload/network/Socket.hh"

#include <common/Array.hh>
#include <formats/ClientState.hh>
#include <formats/ServerState.hh>
#include <jsystem/JKRHeap.hh>

class ConnectionState {
public:
    ConnectionState(JKRHeap *heap, DH::PK serverPK);
    virtual ~ConnectionState();
    virtual ConnectionState &reset() = 0;
    virtual ConnectionState &read(ServerStateReader &reader, u8 *buffer, u32 size,
            const Socket::Address &address, bool &ok) = 0;
    virtual ConnectionState &write(ClientStateWriter &writer, u8 *buffer, u32 &size,
            Socket::Address &address, bool &ok) = 0;

protected:
    JKRHeap *m_heap;
    DH::PK m_serverPK;
};
