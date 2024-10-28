extern "C" {
#include "IPRoute.h"
}

#include <common/Log.hh>

extern "C" void IPGetMacAddr(const IPInterface *interface, u8 macAddr[6]) {
    DEBUG("IPGetMacAddr %p %p", interface, macAddr);
    REPLACED(IPGetMacAddr)(interface, macAddr);
}

extern "C" void IPGetNetmask(const IPInterface *interface, u8 netmask[4]) {
    DEBUG("IPGetNetmask %p %p", interface, netmask);
    REPLACED(IPGetNetmask)(interface, netmask);
}

extern "C" void IPGetAddr(const IPInterface *interface, u8 addr[4]) {
    DEBUG("IPGetAddr %p %p", interface, addr);
    REPLACED(IPGetAddr)(interface, addr);
}

extern "C" void IPGetAlias(const IPInterface *interface, u8 alias[4]) {
    DEBUG("IPGetAlias %p %p", interface, alias);
    REPLACED(IPGetAlias)(interface, alias);
}

extern "C" void IPGetLinkState(const IPInterface *interface, s32 *state) {
    DEBUG("IPGetLinkState %p %p", interface, state);
    REPLACED(IPGetLinkState)(interface, state);
}

extern "C" s32 IPGetConfigError(const IPInterface *interface) {
    DEBUG("IPGetConfigError %p", interface);
    return REPLACED(IPGetConfigError)(interface);
}

extern "C" s32 IPClearConfigError(IPInterface *interface) {
    DEBUG("IPClearConfigError %p", interface);
    return REPLACED(IPClearConfigError)(interface);
}
