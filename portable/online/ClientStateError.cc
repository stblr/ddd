#include "ClientStateError.hh"

ClientStateError::ClientStateError(const ClientPlatform &platform) : ClientState(platform) {
    platform.socket.close();
}

ClientStateError::~ClientStateError() {}

bool ClientStateError::needsSockets() {
    return false;
}

ClientState &ClientStateError::read(ClientReadHandler &handler) {
    handler.clientStateError();
    return *this;
}

ClientState &ClientStateError::writeStateIdle() {
    return *this;
}

ClientState &ClientStateError::writeStateServer() {
    return *this;
}

ClientState &ClientStateError::writeStateRoom() {
    return *this;
}

ClientState &ClientStateError::writeStateError() {
    return *this;
}
