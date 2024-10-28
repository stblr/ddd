extern "C" {
#include "IPSocket.h"
}

#include <common/Log.hh>

extern "C" void SOInit(void) {
    DEBUG("SOInit");
    REPLACED(SOInit)();
}

extern "C" s32 SOStartup(SOConfig *config) {
    DEBUG("SOStartup %p", config);
    return REPLACED(SOStartup)(config);
}

extern "C" s32 SOCleanup(void) {
    DEBUG("SOCleanup");
    return REPLACED(SOCleanup)();
}

extern "C" s32 SOSocket(s32 domain, s32 type, s32 protocol) {
    DEBUG("SOSocket %d %d %d", domain, type, protocol);
    return REPLACED(SOSocket)(domain, type, protocol);
}

extern "C" s32 SOClose(s32 socket) {
    DEBUG("SOClose %d", socket);
    return REPLACED(SOClose)(socket);
}

extern "C" s32 SOListen(s32 socket, s32 backlog) {
    DEBUG("SOListen %d %d", socket, backlog);
    return REPLACED(SOListen)(socket, backlog);
}

extern "C" s32 SOAccept(s32 socket, SOSockAddr *address) {
    DEBUG("SOAccept %d %p", socket, address);
    return REPLACED(SOAccept)(socket, address);
}

extern "C" s32 SOBind(s32 socket, const SOSockAddr *address) {
    DEBUG("SOBind %d %p", socket, address);
    return REPLACED(SOBind)(socket, address);
}

extern "C" s32 SOShutdown(s32 socket, s32 how) {
    DEBUG("SOShutdown %d %d", socket, how);
    return REPLACED(SOShutdown)(socket, how);
}

extern "C" s32 SORecvFrom(s32 socket, void *buffer, s32 length, s32 flags, SOSockAddr *address) {
    DEBUG("SORecvFrom %d %p %d %d %p", socket, buffer, length, flags, address);
    return REPLACED(SORecvFrom)(socket, buffer, length, flags, address);
}

extern "C" s32 SOSendTo(s32 socket, const void *buffer, s32 length, s32 flags,
        const SOSockAddr *address) {
    DEBUG("SOSendTo %d %p %d %d %p", socket, buffer, length, flags, address);
    return REPLACED(SOSendTo)(socket, buffer, length, flags, address);
}

extern "C" s32 SOSetSockOpt(s32 socket, s32 level, s32 optname, const void *optval, s32 optlen) {
    DEBUG("SOSetSockOpt %d %d %d %p %d", socket, level, optname, optval, optlen);
    return REPLACED(SOSetSockOpt)(socket, level, optname, optval, optlen);
}

extern "C" s32 SOFcntl(s32 socket, s32 cmd, ...) {
    DEBUG("SOFcntl %d %d", socket, cmd);
    if (cmd == SO_F_SETFL) {
        va_list vlist;
        va_start(vlist, cmd);
        s32 arg = va_arg(vlist, s32);
        va_end(vlist);
        return REPLACED(SOFcntl)(socket, cmd, arg);
    } else {
        return REPLACED(SOFcntl)(socket, cmd);
    }
}

extern "C" s32 SOPoll(SOPollFD *fds, u32 nfds, s64 timeout) {
    DEBUG("SOPoll %p %u %lld", fds, nfds, timeout);
    return REPLACED(SOPoll)(fds, nfds, timeout);
}
