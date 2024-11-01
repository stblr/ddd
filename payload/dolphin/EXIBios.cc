extern "C" {
#include "EXIBios.h"
}

#include <common/Platform.hh>

struct EXIControl {
    u8 _00[0x20 - 0x00];
    u32 _20;
    u8 _24[0x40 - 0x24];
};
size_assert(EXIControl, 0x40);

extern "C" u32 exiProbeStartTimes[2];
extern "C" EXIControl Ecb[];

extern "C" BOOL __EXIProbe(s32 chan);

extern "C" s32 EXIGetType(s32 chan, u32 dev, u32 *type) {
    if (!Platform::IsGameCube()) {
        if (chan == 0 && dev == 2) {
            *type = 0x04020200;
            return 1;
        }
    }

    return REPLACED(EXIGetType)(chan, dev, type);
}

extern "C" void EXIProbeReset(void) {
    exiProbeStartTimes[0] = 0;
    exiProbeStartTimes[1] = 0;
    Ecb[0]._20 = 0;
    Ecb[1]._20 = 0;
    __EXIProbe(0);
    __EXIProbe(1);
}
