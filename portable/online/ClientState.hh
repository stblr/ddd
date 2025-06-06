#pragma once

#include "portable/online/ClientPlatform.hh"
#include "portable/online/ClientReadHandler.hh"

class ClientState {
public:
    ClientState(const ClientPlatform &platform);
    virtual ~ClientState();
    virtual bool needsSockets() = 0;
    virtual ClientState &read(ClientReadHandler &handler) = 0;
    virtual ClientState &writeStateIdle();
    virtual ClientState &writeStateServer();
    virtual ClientState &writeStateRoom();
    virtual ClientState &writeStateError();

protected:
    const ClientPlatform &m_platform;
};
