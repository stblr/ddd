extern "C" {
#include "IPRoute.h"

#include "dolphin/IPSocket.h"
}

#include <common/Log.hh>
#include <common/Platform.hh>

extern "C" {
#include <string.h>
}

extern "C" void IPGetMacAddr(const IPInterface *interface, u8 macAddr[6]) {
    DEBUG("IPGetMacAddr %p %p", interface, macAddr);
    if (Platform::IsGameCube()) {
        REPLACED(IPGetMacAddr)(interface, macAddr);
        return;
    }

    memset(macAddr, 0, 6);
    s32 optlen = 6;
    SOGetInterfaceOpt(SO_CONFIG_MAC_ADDRESS, macAddr, &optlen);
}

extern "C" void IPGetNetmask(const IPInterface *interface, u8 netmask[4]) {
    DEBUG("IPGetNetmask %p %p", interface, netmask);
    if (Platform::IsGameCube()) {
        REPLACED(IPGetNetmask)(interface, netmask);
        return;
    }

    u8 table[12];
    memset(table, 0, sizeof(table));
    s32 optlen = sizeof(table);
    SOGetInterfaceOpt(SO_CONFIG_IP_ADDR_TABLE, table, &optlen);
    memcpy(netmask, table + 4, 4);
}

extern "C" void IPGetAddr(const IPInterface *interface, u8 addr[4]) {
    DEBUG("IPGetAddr %p %p", interface, addr);
    if (Platform::IsGameCube()) {
        REPLACED(IPGetAddr)(interface, addr);
        return;
    }

    u8 table[12];
    memset(table, 0, sizeof(table));
    s32 optlen = sizeof(table);
    SOGetInterfaceOpt(SO_CONFIG_IP_ADDR_TABLE, table, &optlen);
    memcpy(addr, table + 0, 4);
}

extern "C" void IPGetAlias(const IPInterface *interface, u8 alias[4]) {
    DEBUG("IPGetAlias %p %p", interface, alias);
    if (Platform::IsGameCube()) {
        REPLACED(IPGetAlias)(interface, alias);
        return;
    }

    u8 table[24];
    memset(table, 0, sizeof(table));
    s32 optlen = sizeof(table);
    SOGetInterfaceOpt(SO_CONFIG_IP_ADDR_TABLE, table, &optlen);
    if (optlen == sizeof(table)) {
        memcpy(alias, table + 12, 4);
    } else {
        memcpy(alias, table + 0, 4);
    }
}

extern "C" void IPGetLinkState(const IPInterface *interface, s32 *state) {
    DEBUG("IPGetLinkState %p %p", interface, state);
    if (Platform::IsGameCube()) {
        REPLACED(IPGetLinkState)(interface, state);
        return;
    }

    *state = 0;
    s32 optlen = sizeof(*state);
    SOGetInterfaceOpt(SO_CONFIG_LINK_STATE, state, &optlen);
    DEBUG("%d", *state);
}

extern "C" s32 IPGetConfigError(const IPInterface *interface) {
    DEBUG("IPGetConfigError %p", interface);
    if (Platform::IsGameCube()) {
        return REPLACED(IPGetConfigError)(interface);
    }

    s32 configError = 0;
    s32 optlen = sizeof(configError);
    SOGetInterfaceOpt(SO_CONFIG_ERROR, &configError, &optlen);
    return configError;
}

extern "C" s32 IPClearConfigError(IPInterface *interface) {
    DEBUG("IPClearConfigError %p", interface);
    if (Platform::IsGameCube()) {
        return REPLACED(IPClearConfigError)(interface);
    }

    s32 prevConfigError = IPGetConfigError(interface);
    s32 configError = 0;
    SOSetInterfaceOpt(SO_CONFIG_ERROR, &configError, sizeof(configError));
    return prevConfigError;
}
