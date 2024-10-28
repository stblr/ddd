#pragma once

#include <common/Types.h>
#include <payload/Replace.h>

enum {
    SO_F_GETFL = 3,
    SO_F_SETFL = 4,
};

typedef struct SOConfig SOConfig;
typedef struct SOSockAddr SOSockAddr;
typedef struct SOPollFD SOPollFD;

void REPLACED(SOInit)(void);
REPLACE void SOInit(void);
s32 REPLACED(SOStartup)(SOConfig *config);
REPLACE s32 SOStartup(SOConfig *config);
s32 REPLACED(SOCleanup)(void);
REPLACE s32 SOCleanup(void);
s32 REPLACED(SOSocket)(s32 domain, s32 type, s32 protocol);
REPLACE s32 SOSocket(s32 domain, s32 type, s32 protocol);
s32 REPLACED(SOClose)(s32 socket);
REPLACE s32 SOClose(s32 socket);
s32 REPLACED(SOListen)(s32 socket, s32 backlog);
REPLACE s32 SOListen(s32 socket, s32 backlog);
s32 REPLACED(SOAccept)(s32 socket, SOSockAddr *address);
REPLACE s32 SOAccept(s32 socket, SOSockAddr *address);
s32 REPLACED(SOBind)(s32 socket, const SOSockAddr *address);
REPLACE s32 SOBind(s32 socket, const SOSockAddr *address);
s32 REPLACED(SOShutdown)(s32 socket, s32 how);
REPLACE s32 SOShutdown(s32 socket, s32 how);
s32 REPLACED(SORecvFrom)(s32 socket, void *buffer, s32 length, s32 flags, SOSockAddr *address);
REPLACE s32 SORecvFrom(s32 socket, void *buffer, s32 length, s32 flags, SOSockAddr *address);
// clang-format off
s32 REPLACED(SOSendTo)(s32 socket, const void *buffer, s32 length, s32 flags,
        const SOSockAddr *address);
// clang-format on
REPLACE s32 SOSendTo(s32 socket, const void *buffer, s32 length, s32 flags,
        const SOSockAddr *address);
s32 REPLACED(SOSetSockOpt)(s32 socket, s32 level, s32 optname, const void *optval, s32 optlen);
REPLACE s32 SOSetSockOpt(s32 socket, s32 level, s32 optname, const void *optval, s32 optlen);
s32 REPLACED(SOFcntl)(s32 socket, s32 cmd, ...);
REPLACE s32 SOFcntl(s32 socket, s32 cmd, ...);
s32 REPLACED(SOPoll)(SOPollFD *fds, u32 nfds, s64 timeout);
REPLACE s32 SOPoll(SOPollFD *fds, u32 nfds, s64 timeout);
