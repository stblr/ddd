extern "C" {
#include "OSRtc.h"
}

#include <payload/Replace.hh>
#include <portable/Bytes.hh>
#include <portable/Log.hh>

extern "C" u8 *__OSLockSram();
extern "C" u8 *REPLACED(__OSLockSramEx)();
extern "C" void __OSUnlockSram(BOOL write);

extern "C" REPLACE u8 *__OSLockSramEx() {
    INFO("*__OSLockSramEx");
    u8 *result = REPLACED(__OSLockSramEx)();
    INFO("*__OSLockSramEx %p", result);
    return result;
}

extern "C" u32 OSGetCounterBias() {
    u8 *sram = __OSLockSram();
    u32 counterBias = Bytes::ReadBE<u32>(sram, 0x0c);
    __OSUnlockSram(false);
    return counterBias;
}
