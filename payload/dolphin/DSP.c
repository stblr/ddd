#include "DSP.h"

#include <portable/Log.h>

static void Relocate(u32 *addr) {
    if (*addr < 0x00400000) {
        *addr += 0x400000;
    }
}

DSPTaskInfo *DSPAddTask(DSPTaskInfo *taskInfo) {
    INFO("DSPAddTask %p %x %x", taskInfo, taskInfo->iramMMEMAddr, taskInfo->dramMMEMAddr);
    Relocate(&taskInfo->iramMMEMAddr);
    Relocate(&taskInfo->dramMMEMAddr);
    INFO("DSPAddTask %p %x %x", taskInfo, taskInfo->iramMMEMAddr, taskInfo->dramMMEMAddr);

    DSPTaskInfo *result = REPLACED(DSPAddTask)(taskInfo);
    return result;
}

void DSPAddPriorTask(DSPTaskInfo *taskInfo) {
    Relocate(&taskInfo->iramMMEMAddr);
    Relocate(&taskInfo->dramMMEMAddr);

    REPLACED(DSPAddPriorTask)(taskInfo);
}
