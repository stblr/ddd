extern "C" {
#include "CARD.h"
}

#include <payload/VirtualCard.hh>
#include <portable/Log.hh>

static Array<VirtualCard *, CARD_NUM_CHANS> s_virtualCards(nullptr);
static Array<VirtualCard *, CARD_NUM_CHANS> s_nextVirtualCards(nullptr);
bool b = false;

extern "C" s32 REPLACED(__CARDReadStatus)(s32 chan, u8 *status);
extern "C" REPLACE s32 __CARDReadStatus(s32 chan, u8 *status) {
    INFO("CARDReadStatus %d %p", chan, status);
    s32 result = REPLACED(__CARDReadStatus)(chan, status);
    INFO("CARDReadStatus %d %p %u %d", chan, status, *status, result);
    return result;
}

extern "C" s32 CARDFreeBlocks(s32 chan, s32 *bytesNotUsed, s32 *filesNotUsed) {
    if (s_virtualCards[chan]) {
        return s_virtualCards[chan]->freeBlocks(bytesNotUsed, filesNotUsed);
    }

    return REPLACED(CARDFreeBlocks)(chan, bytesNotUsed, filesNotUsed);
}

extern "C" s32 CARDCheck(s32 chan) {
    if (s_virtualCards[chan]) {
        return s_virtualCards[chan]->check();
    }

    return REPLACED(CARDCheck)(chan);
}

extern "C" s32 CARDProbeEx(s32 chan, s32 *memSize, s32 *sectorSize) {
    /*if (b)
    INFO("CARDProbeEx %d %p %p", chan, memSize, sectorSize);*/
    s32 result = REPLACED(CARDProbeEx)(chan, memSize, sectorSize);
    if (b)
    INFO("CARDProbeEx %d %d", chan, result);
    if (result == CARD_RESULT_READY) {
        s_nextVirtualCards[chan] = nullptr;
        return CARD_RESULT_READY;
    }

    s_nextVirtualCards[chan] = VirtualCard::Instance(chan);
    result = s_nextVirtualCards[chan]->probeEx(memSize, sectorSize);
    if (b)
    INFO("CARDProbeEx (virtual) %d %d", chan, result);
    return result;
}

extern "C" s32 CARDMount(s32 chan, void *workArea, CARDCallback detachCallback) {
    b = true;
    s_virtualCards[chan] = s_nextVirtualCards[chan];

    if (s_virtualCards[chan]) {
        return s_virtualCards[chan]->mount(workArea, detachCallback);
    }

    INFO("CARDMount %d %p %p", chan, workArea, detachCallback);
    s32 result = REPLACED(CARDMount)(chan, workArea, detachCallback);
    INFO("CARDMount %d", result);
    return result;
}

extern "C" s32 REPLACED(CARDMountAsync)(s32 chan, void *workArea, CARDCallback detachCallback, void *ptr);
extern "C" REPLACE s32 CARDMountAsync(s32 chan, void *workArea, CARDCallback detachCallback, void *ptr) {
    INFO("CARDMountAsync %d %p %p %p", chan, workArea, detachCallback, ptr);
    s32 result = REPLACED(CARDMountAsync)(chan, workArea, detachCallback, ptr);
    INFO("CARDMountAsync %d", result);
    return result;
}

extern "C" s32 CARDUnmount(s32 chan) {
    if (s_virtualCards[chan]) {
        return s_virtualCards[chan]->unmount();
    }

    return REPLACED(CARDUnmount)(chan);
}

extern "C" s32 CARDFormat(s32 chan) {
    if (s_virtualCards[chan]) {
        return s_virtualCards[chan]->format();
    }

    return REPLACED(CARDFormat)(chan);
}

extern "C" s32 CARDFastOpen(s32 chan, s32 fileNo, CARDFileInfo *fileInfo) {
    if (s_virtualCards[chan]) {
        return s_virtualCards[chan]->fastOpen(fileNo, fileInfo);
    }

    s32 result = REPLACED(CARDFastOpen)(chan, fileNo, fileInfo);
    DEBUG("CARDFastOpen %d", result);
    return result;
}

extern "C" s32 CARDOpen(s32 chan, const char *fileName, CARDFileInfo *fileInfo) {
    if (s_virtualCards[chan]) {
        return s_virtualCards[chan]->open(fileName, fileInfo);
    }

    s32 result = REPLACED(CARDOpen)(chan, fileName, fileInfo);
    DEBUG("CARDOpen %d", result);
    return result;
}

extern "C" s32 CARDClose(CARDFileInfo *fileInfo) {
    if (s_virtualCards[fileInfo->chan]) {
        return s_virtualCards[fileInfo->chan]->close(fileInfo);
    }

    return REPLACED(CARDClose)(fileInfo);
}

extern "C" s32 CARDCreate(s32 chan, const char *fileName, u32 size, CARDFileInfo *fileInfo) {
    if (s_virtualCards[chan]) {
        return s_virtualCards[chan]->create(fileName, size, fileInfo);
    }

    return REPLACED(CARDCreate)(chan, fileName, size, fileInfo);
}

extern "C" s32 CARDRead(CARDFileInfo *fileInfo, void *addr, s32 length, s32 offset) {
    if (s_virtualCards[fileInfo->chan]) {
        return s_virtualCards[fileInfo->chan]->read(fileInfo, addr, length, offset);
    }

    s32 result = REPLACED(CARDRead)(fileInfo, addr, length, offset);
    DEBUG("CARDRead %d", result);
    return result;
}

extern "C" s32 CARDWrite(CARDFileInfo *fileInfo, const void *addr, s32 length, s32 offset) {
    if (s_virtualCards[fileInfo->chan]) {
        return s_virtualCards[fileInfo->chan]->write(fileInfo, addr, length, offset);
    }

    return REPLACED(CARDWrite)(fileInfo, addr, length, offset);
}

extern "C" s32 CARDFastDelete(s32 chan, s32 fileNo) {
    if (s_virtualCards[chan]) {
        return s_virtualCards[chan]->fastRemove(fileNo);
    }

    return REPLACED(CARDFastDelete)(chan, fileNo);
}

extern "C" s32 CARDDelete(s32 chan, const char *fileName) {
    if (s_virtualCards[chan]) {
        return s_virtualCards[chan]->remove(fileName);
    }

    return REPLACED(CARDDelete)(chan, fileName);
}

extern "C" s32 CARDGetStatus(s32 chan, s32 fileNo, CARDStat *stat) {
    if (s_virtualCards[chan]) {
        return s_virtualCards[chan]->getStatus(fileNo, stat);
    }

    s32 result = REPLACED(CARDGetStatus)(chan, fileNo, stat);
    DEBUG("CARDGetStatus %d", result);
    return result;
}

extern "C" s32 CARDSetStatus(s32 chan, s32 fileNo, CARDStat *stat) {
    if (s_virtualCards[chan]) {
        return s_virtualCards[chan]->setStatus(fileNo, stat);
    }

    return REPLACED(CARDSetStatus)(chan, fileNo, stat);
}

extern "C" s32 CARDRename(s32 chan, const char *oldName, const char *newName) {
    if (s_virtualCards[chan]) {
        return s_virtualCards[chan]->rename(oldName, newName);
    }

    return REPLACED(CARDRename)(chan, oldName, newName);
}
