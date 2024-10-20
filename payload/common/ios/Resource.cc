#include <common/ios/Resource.hh>

#include <common/DCache.hh>
#include <common/Memory.hh>
extern "C" {
#include <dolphin/OSInterrupt.h>
#include <dolphin/OSThread.h>
}
#include <payload/Lock.hh>

enum {
    X1 = 1 << 0,
    Y2 = 1 << 1,
    Y1 = 1 << 2,
    X2 = 1 << 3,
    IY1 = 1 << 4,
    IY2 = 1 << 5,
};

extern "C" volatile u32 ppcmsg;
extern "C" volatile u32 ppcctrl;
extern "C" volatile u32 armmsg;
extern "C" volatile u32 ppcirqflag;

namespace IOS {

void Resource::Init() {
    OSSetInterruptHandler(27, HandleInterrupt);
    OSUnmaskInterrupts(1 << (31 - 27));
    ppcctrl = IY2 | IY1 | X2;
}

void Resource::Sync(Request &request) {
    DCache::Flush(&request, offsetof(Request, user));

    Lock<NoInterrupts> lock;
    ppcmsg = Memory::VirtualToPhysical(&request);
    ppcctrl = IY2 | IY1 | X1;

    OSThreadQueue *queue = reinterpret_cast<OSThreadQueue *>(request.user);
    OSInitThreadQueue(queue);
    OSSleepThread(queue);
}

void Resource::HandleInterrupt(s16 /* interrupt */, OSContext * /* context */) {
    if (ppcctrl & Y1) {
        ppcctrl = IY2 | IY1 | Y1;
        ppcirqflag = 1 << 30;

        Request *reply = Memory::PhysicalToVirtual<Request>(armmsg);
        DCache::Invalidate(reply, offsetof(Request, user));
        OSThreadQueue *queue = reinterpret_cast<OSThreadQueue *>(reply->user);
        OSWakeupThread(queue);

        ppcctrl = IY2 | IY1 | X2;
    }

    if (ppcctrl & Y2) {
        ppcctrl = IY2 | IY1 | Y2;
        ppcirqflag = 1 << 30;
    }
}

} // namespace IOS
