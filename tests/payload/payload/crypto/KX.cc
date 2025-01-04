#include <lest.hpp>
extern "C" {
#include <monocypher/monocypher.h>
}
#include <payload/crypto/KX.hh>
#include <payload/crypto/Random.hh>

static lest::tests specification;

#define CASE(name) lest_CASE(specification, name)

CASE("Invalid server PK") {
    DH::K clientK, serverK, invalidServerK;
    DH::PK invalidServerPK(invalidServerK);

    KX::ClientState clientState(clientK, invalidServerPK);
    std::array<u8, KX::M1Size> m1;
    while (!clientState.getM1(m1.data())) {
        EXPECT(clientState.update());
    }
    KX::ServerState serverState(serverK);
    EXPECT(serverState.setM1(m1.data()));
    while (serverState.update()) {}
    EXPECT_NOT(serverState.hasM2());
    EXPECT_NOT(serverState.serverSession());
}

CASE("Invalid m1") {
    DH::K clientK, serverK;
    DH::PK serverPK(serverK);

    KX::ClientState clientState(clientK, serverPK);
    while (!clientState.hasM1()) {
        EXPECT(clientState.update());
    }
    KX::ServerState serverState(serverK);
    std::array<u8, KX::M1Size> invalidM1;
    Random::Get(invalidM1.data(), invalidM1.size());
    EXPECT(serverState.setM1(invalidM1.data()));
    while (serverState.update()) {}
    EXPECT_NOT(serverState.hasM2());
    EXPECT_NOT(serverState.serverSession());
}

CASE("Invalid m2") {
    DH::K clientK, serverK;
    DH::PK serverPK(serverK);

    KX::ClientState clientState(clientK, serverPK);
    while (!clientState.hasM1()) {
        EXPECT(clientState.update());
    }
    std::array<u8, KX::M2Size> invalidM2;
    Random::Get(invalidM2.data(), invalidM2.size());
    EXPECT(clientState.setM2(invalidM2.data()));
    while (clientState.update()) {}
    EXPECT_NOT(clientState.clientSession());
}

CASE("Valid") {
    DH::K clientK, serverK;
    DH::PK serverPK(serverK);

    KX::ClientState clientState(clientK, serverPK);
    std::array<u8, KX::M1Size> m1;
    while (!clientState.getM1(m1.data())) {
        EXPECT(clientState.update());
    }
    KX::ServerState serverState(serverK);
    EXPECT(serverState.setM1(m1.data()));
    while (serverState.update()) {}
    std::array<u8, KX::M2Size> m2;
    EXPECT(serverState.getM2(m2.data()));
    const DH::PK *clientPK = serverState.clientPK();
    EXPECT(clientPK);
    const Session *serverSession = serverState.serverSession();
    EXPECT(serverSession);
    EXPECT(clientState.setM2(m2.data()));
    while (clientState.update()) {}
    const Session *clientSession = clientState.clientSession();
    EXPECT(clientSession);

    DH::PK expectedClientPK(clientK);
    EXPECT(clientPK->m_q == expectedClientPK.m_q);
    EXPECT(serverSession->m_readK != serverSession->m_writeK);
    EXPECT(serverSession->m_readK == clientSession->m_writeK);
    EXPECT(serverSession->m_writeK == clientSession->m_readK);
}

int main(int argc, char *argv[]) {
    return lest::run(specification, argc, argv, std::cerr);
}
