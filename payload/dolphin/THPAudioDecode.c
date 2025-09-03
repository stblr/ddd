#include <payload/Replace.h>
#include <portable/Log.h>

void REPLACED(AudioDecodeThreadCancel)();
REPLACE void AudioDecodeThreadCancel() {
    DEBUG("audio decode");

    REPLACED(AudioDecodeThreadCancel)();

    DEBUG("audio decode");
}
