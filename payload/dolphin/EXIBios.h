#pragma once

#include <common/Types.h>
#include <payload/Replace.h>

enum {
    EXI_READ = 0,
    EXI_WRITE = 1,
};

BOOL EXIImmEx(s32 chan, void *buf, s32 len, u32 type);
BOOL EXISelect(s32 chan, u32 dev, u32 freq);
BOOL EXIDeselect(s32 chan);
BOOL EXILock(s32 chan, u32 dev, void *unlockedCallback);
BOOL EXIUnlock(s32 chan);
s32 REPLACED(EXIGetType)(s32 chan, u32 dev, u32 *type);
REPLACE s32 EXIGetType(s32 chan, u32 dev, u32 *type);
void EXIProbeReset(void);
