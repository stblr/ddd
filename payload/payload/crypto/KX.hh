#pragma once

#include "payload/crypto/DH.hh"
#include "payload/crypto/Session.hh"
#include "payload/crypto/SymmetricState.hh"

#include <common/Array.hh>
#include <common/Optional.hh>

class KX {
public:
    enum {
        M1Size = 96,
        M2Size = 48,
    };

    class ClientState {
    public:
        ClientState(DH::K clientK, DH::PK serverPK);
        ~ClientState();
        bool hasM1() const;
        bool hasM2() const;
        bool getM1(u8 m1[M1Size]) const;
        bool setM2(const u8 m2[M2Size]);
        bool update();
        const Session *clientSession() const;

    private:
        typedef void (ClientState::*State)();

        void statePrologue();
        void stateS();
        void stateM1E();
        void stateM1ES();
        void stateM1S();
        void stateM1SS();
        void stateM1Final();
        void stateM2E();
        void stateM2EE();
        void stateM2SE();
        void stateM2Final();
        void stateSession();

        bool m_hasM1;
        bool m_hasM2;
        bool m_hasClientSession;
        Array<u8, M1Size> m_m1;
        Array<u8, M2Size> m_m2;
        Session m_clientSession;
        State m_state;
        SymmetricState m_symmetricState;
        Optional<DH::K> m_clientK;
        Optional<DH::K> m_clientEphemeralK;
        DH::PK m_serverPK;
        DH::PK m_serverEphemeralPK;
    };

    class ServerState {
    public:
        ServerState(DH::K serverK);
        ~ServerState();
        bool hasM1() const;
        bool hasM2() const;
        bool setM1(const u8 m1[M1Size]);
        bool getM2(u8 m2[M2Size]) const;
        bool update();
        const Session *serverSession() const;
        const DH::PK *clientPK() const;

    private:
        typedef void (ServerState::*State)();

        void statePrologue();
        void stateS();
        void stateM1E();
        void stateM1ES();
        void stateM1S();
        void stateM1SS();
        void stateM1Final();
        void stateM2E();
        void stateM2EE();
        void stateM2SE();
        void stateM2Final();
        void stateSession();

        bool m_hasM1;
        bool m_hasM2;
        bool m_hasServerSession;
        Array<u8, M1Size> m_m1;
        Array<u8, M2Size> m_m2;
        Session m_serverSession;
        State m_state;
        SymmetricState m_symmetricState;
        DH::PK m_clientPK;
        DH::PK m_clientEphemeralPK;
        Optional<DH::K> m_serverK;
        Optional<DH::K> m_serverEphemeralK;
    };

private:
    KX();
};
