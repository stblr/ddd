// clang-format off
//
// Based on https://github.com/blckngm/noise-rust/blob/master/noise-protocol/src/handshakestate.rs
//
// clang-format on

#include "KX.hh"

extern "C" {
#include <monocypher/monocypher.h>

#include <string.h>
}

KX::ClientState::ClientState(DH::K clientK, DH::PK serverPK)
    : m_hasM1(false), m_hasM2(false), m_hasClientSession(false),
      m_state(&ClientState::statePrologue), m_clientK(clientK), m_serverPK(serverPK) {}

KX::ClientState::~ClientState() {}

bool KX::ClientState::hasM1() const {
    return m_hasM1;
}

bool KX::ClientState::hasM2() const {
    return m_hasM2;
}

bool KX::ClientState::getM1(u8 m1[M1Size]) const {
    if (!m_hasM1) {
        return false;
    }

    memcpy(m1, m_m1.values(), M1Size);
    return true;
}

bool KX::ClientState::setM2(const u8 m2[M2Size]) {
    if (!m_hasM1 || m_hasM2) {
        return false;
    }

    memcpy(m_m2.values(), m2, M2Size);
    m_hasM2 = true;
    return true;
}

bool KX::ClientState::update() {
    (this->*m_state)();
    return m_state;
}

const Session *KX::ClientState::clientSession() const {
    return m_hasClientSession ? &m_clientSession : static_cast<Session *>(nullptr);
}

void KX::ClientState::statePrologue() {
    m_symmetricState.mixHash(nullptr, 0);
    m_state = &ClientState::stateS;
}

void KX::ClientState::stateS() {
    m_symmetricState.mixHash(m_serverPK.m_q.values(), m_serverPK.m_q.count());
    m_state = &ClientState::stateM1E;
}

void KX::ClientState::stateM1E() {
    m_clientEphemeralK.emplace();
    br_ec_compute_pub(&br_ec_c25519_m15, nullptr, m_m1.values() + 0, &m_clientEphemeralK->m_k);
    m_symmetricState.mixHash(m_m1.values() + 0, 32);
    m_state = &ClientState::stateM1ES;
}

void KX::ClientState::stateM1ES() {
    m_symmetricState.mixDH(*m_clientEphemeralK, m_serverPK);
    m_state = &ClientState::stateM1S;
}

void KX::ClientState::stateM1S() {
    DH::PK clientPK(*m_clientK);
    m_symmetricState.encryptAndHash(clientPK.m_q.values(), clientPK.m_q.count(),
            m_m1.values() + 32);
    m_state = &ClientState::stateM1SS;
}

void KX::ClientState::stateM1SS() {
    m_symmetricState.mixDH(*m_clientK, m_serverPK);
    m_state = &ClientState::stateM1Final;
}

void KX::ClientState::stateM1Final() {
    m_symmetricState.encryptAndHash(nullptr, 0, m_m1.values() + 80);
    m_hasM1 = true;
    m_state = &ClientState::stateM2E;
}

void KX::ClientState::stateM2E() {
    if (!m_hasM2) {
        return;
    }

    memcpy(m_serverEphemeralPK.m_q.values(), m_m2.values() + 0, m_serverEphemeralPK.m_q.count());
    m_symmetricState.mixHash(m_m2.values() + 0, 32);
    m_state = &ClientState::stateM2EE;
}

void KX::ClientState::stateM2EE() {
    m_symmetricState.mixDH(*m_clientEphemeralK, m_serverEphemeralPK);
    m_clientEphemeralK.reset();
    m_state = &ClientState::stateM2SE;
}

void KX::ClientState::stateM2SE() {
    m_symmetricState.mixDH(*m_clientK, m_serverEphemeralPK);
    m_clientK.reset();
    m_state = &ClientState::stateM2Final;
}

void KX::ClientState::stateM2Final() {
    if (!m_symmetricState.decryptAndHash(m_m2.values() + 32, nullptr, 0)) {
        m_state = static_cast<State>(nullptr);
        return;
    }

    m_state = &ClientState::stateSession;
}

void KX::ClientState::stateSession() {
    m_symmetricState.computeSessionKeys(m_clientSession.m_writeK, m_clientSession.m_readK);
    m_hasClientSession = true;
    m_state = static_cast<State>(nullptr);
}

KX::ServerState::ServerState(DH::K serverK)
    : m_hasM1(false), m_hasM2(false), m_hasServerSession(false),
      m_state(&ServerState::statePrologue), m_serverK(serverK) {}

KX::ServerState::~ServerState() {}

bool KX::ServerState::hasM1() const {
    return m_hasM1;
}

bool KX::ServerState::hasM2() const {
    return m_hasM2;
}

bool KX::ServerState::setM1(const u8 m1[M1Size]) {
    if (m_hasM1) {
        return false;
    }

    memcpy(m_m1.values(), m1, M1Size);
    m_hasM1 = true;
    return true;
}

bool KX::ServerState::getM2(u8 m2[M2Size]) const {
    if (!m_hasM2) {
        return false;
    }

    memcpy(m2, m_m2.values(), M2Size);
    return true;
}

bool KX::ServerState::update() {
    (this->*m_state)();
    return m_state;
}

const Session *KX::ServerState::serverSession() const {
    return m_hasServerSession ? &m_serverSession : static_cast<Session *>(nullptr);
}

const DH::PK *KX::ServerState::clientPK() const {
    return m_hasServerSession ? &m_clientPK : static_cast<DH::PK *>(nullptr);
}

void KX::ServerState::statePrologue() {
    m_symmetricState.mixHash(nullptr, 0);
    m_state = &ServerState::stateS;
}

void KX::ServerState::stateS() {
    DH::PK serverPK(*m_serverK);
    m_symmetricState.mixHash(serverPK.m_q.values(), serverPK.m_q.count());
    m_state = &ServerState::stateM1E;
}

void KX::ServerState::stateM1E() {
    if (!m_hasM1) {
        return;
    }

    memcpy(m_clientEphemeralPK.m_q.values(), m_m1.values() + 0, m_clientEphemeralPK.m_q.count());
    m_symmetricState.mixHash(m_m1.values(), 32);
    m_state = &ServerState::stateM1ES;
}

void KX::ServerState::stateM1ES() {
    m_symmetricState.mixDH(*m_serverK, m_clientEphemeralPK);
    m_state = &ServerState::stateM1S;
}

void KX::ServerState::stateM1S() {
    if (!m_symmetricState.decryptAndHash(m_m1.values() + 32, m_clientPK.m_q.values(),
                m_clientPK.m_q.count())) {
        m_state = static_cast<State>(nullptr);
        return;
    }

    m_state = &ServerState::stateM1SS;
}

void KX::ServerState::stateM1SS() {
    m_symmetricState.mixDH(*m_serverK, m_clientPK);
    m_serverK.reset();
    m_state = &ServerState::stateM1Final;
}

void KX::ServerState::stateM1Final() {
    if (!m_symmetricState.decryptAndHash(m_m1.values() + 80, nullptr, 0)) {
        m_state = static_cast<State>(nullptr);
        return;
    }

    m_state = &ServerState::stateM2E;
}

void KX::ServerState::stateM2E() {
    m_serverEphemeralK.emplace();
    br_ec_compute_pub(&br_ec_c25519_m15, nullptr, m_m2.values() + 0, &m_serverEphemeralK->m_k);
    m_symmetricState.mixHash(m_m2.values() + 0, 32);
    m_state = &ServerState::stateM2EE;
}

void KX::ServerState::stateM2EE() {
    m_symmetricState.mixDH(*m_serverEphemeralK, m_clientEphemeralPK);
    m_state = &ServerState::stateM2SE;
}

void KX::ServerState::stateM2SE() {
    m_symmetricState.mixDH(*m_serverEphemeralK, m_clientPK);
    m_serverEphemeralK.reset();
    m_state = &ServerState::stateM2Final;
}

void KX::ServerState::stateM2Final() {
    m_symmetricState.encryptAndHash(nullptr, 0, m_m2.values() + 32);
    m_hasM2 = true;
    m_state = &ServerState::stateSession;
}

void KX::ServerState::stateSession() {
    m_symmetricState.computeSessionKeys(m_serverSession.m_readK, m_serverSession.m_writeK);
    m_hasServerSession = true;
    m_state = static_cast<State>(nullptr);
}
