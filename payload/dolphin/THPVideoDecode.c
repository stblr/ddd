#include <payload/Replace.h>
#include <portable/Log.h>

void REPLACED(VideoDecodeThreadCancel)();
REPLACE void VideoDecodeThreadCancel() {
    DEBUG("video decode");

    REPLACED(VideoDecodeThreadCancel)();

    DEBUG("video decode");
}
