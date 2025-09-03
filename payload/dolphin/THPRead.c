#include <payload/Replace.h>
#include <portable/Log.h>

void REPLACED(ReadThreadCancel)();
REPLACE void ReadThreadCancel() {
    DEBUG("read");

    REPLACED(ReadThreadCancel)();

    DEBUG("read");
}
