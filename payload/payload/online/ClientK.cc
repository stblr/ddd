#include "ClientK.hh"

#include <common/Arena.hh>

void ClientK::Init() {
    s_instance = new (MEM1Arena::Instance(), 0x20) DH::K;
}

const DH::K &ClientK::Get() {
    return *s_instance;
}

DH::K *ClientK::s_instance = nullptr;
