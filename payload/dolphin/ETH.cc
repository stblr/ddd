extern "C" {
#include "ETH.h"
}

#include <payload/VirtualETH.hh>
#include <portable/Log.hh>

static VirtualETH *s_virtualETH = nullptr;

s32 ETHInit(s32 mode) {
    DEBUG("ETHInit %d", mode);

    s_virtualETH = VirtualETH::Instance();
    s32 result = s_virtualETH->init(mode);
    if (result >= 0) {
        return result;
    }

    s_virtualETH = nullptr;
    return REPLACED(ETHInit)(mode);
}

void ETHGetMACAddr(u8 *macaddr) {
    DEBUG("ETHGetMACAddr %p", macaddr);

    if (s_virtualETH) {
        s_virtualETH->getMACAddr(macaddr);
        return;
    }

    REPLACED(ETHGetMACAddr)(macaddr);
}

void ETHSetRecvCallback(ETHCallback0 callback0, ETHCallback1 callback1) {
    DEBUG("ETHSetRecvCallback %p %p", callback0, callback1);

    if (s_virtualETH) {
        s_virtualETH->setRecvCallback(callback0, callback1);
        return;
    }

    REPLACED(ETHSetRecvCallback)(callback0, callback1);
}

BOOL ETHGetLinkStateAsync(BOOL *status) {
    if (s_virtualETH) {
        return s_virtualETH->getLinkStateAsync(status);
    }

    return REPLACED(ETHGetLinkStateAsync)(status);
}

void ETHSetProtoType(u16 *array, s32 num) {
    DEBUG("ETHSetProtoType %p %d", array, num);

    if (s_virtualETH) {
        s_virtualETH->setProtoType(array, num);
        return;
    }

    REPLACED(ETHSetProtoType)(array, num);
}

void ETHSendAsync(void *addr, s32 length, ETHCallback2 callback2) {
    DEBUG("ETHSendAsync %p %d %p", addr, length, callback2);

    if (s_virtualETH) {
        s_virtualETH->sendAsync(addr, length, callback2);
        return;
    }

    REPLACED(ETHSendAsync)(addr, length, callback2);
}

void ETHAddMulticastAddress(const u8 macaddr[6]) {
    DEBUG("ETHAddMulticastAddress %p", macaddr);

    if (s_virtualETH) {
        s_virtualETH->addMulticastAddress(macaddr);
        return;
    }

    REPLACED(ETHAddMulticastAddress)(macaddr);
}

void ETHClearMulticastAddresses() {
    DEBUG("ETHClearMulticastAddresses");

    if (s_virtualETH) {
        s_virtualETH->clearMulticastAddresses();
        return;
    }

    REPLACED(ETHClearMulticastAddresses)();
}
