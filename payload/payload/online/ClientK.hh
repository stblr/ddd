#pragma once

#include "payload/crypto/DH.hh"

class ClientK {
public:
    static void Init();
    static const DH::K &Get();

private:
    ClientK();

    static DH::K *s_instance;
};
