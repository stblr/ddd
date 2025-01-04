#pragma once

#include "payload/crypto/DH.hh"
#include "payload/online/ConnectionState.hh"

class ConnectionStateDNS : public ConnectionState {
public:
    ConnectionStateDNS(JKRHeap *heap, DH::PK serverPK, const char *name);
    ~ConnectionStateDNS() override;
    ConnectionState &reset() override;
    ConnectionState &read(ServerStateReader &reader, u8 *buffer, u32 size,
            const Socket::Address &address, bool &ok) override;
    ConnectionState &write(ClientStateWriter &writer, u8 *buffer, u32 &size,
            Socket::Address &address, bool &ok) override;

private:
    Array<char, 256> m_name;
    u16 m_port;
};
