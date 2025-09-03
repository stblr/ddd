#include <payload/Replace.h>
#include <portable/Log.h>

void REPLACED(THPPlayerQuit)();
REPLACE void THPPlayerQuit() {
    DEBUG("quit");

    REPLACED(THPPlayerQuit)();

    DEBUG("quit");
}

void REPLACED(THPPlayerClose)();
REPLACE void THPPlayerClose() {
    DEBUG("close");

    REPLACED(THPPlayerClose)();

    DEBUG("close");
}

void REPLACED(THPPlayerStop)();
REPLACE void THPPlayerStop() {
    DEBUG("stop");

    REPLACED(THPPlayerStop)();

    DEBUG("stop");
}
