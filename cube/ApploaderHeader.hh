#include <portable/Types.hh>

typedef void (*ApploaderReportFunc)(const char *format, ...);
typedef void (*GameEntryFunc)();

typedef void (*ApploaderInitFunc)(ApploaderReportFunc);
typedef s32 (*ApploaderMainFunc)(void **dst, u32 *size, u32 *shiftedOffset);
typedef GameEntryFunc (*ApploaderCloseFunc)();

typedef void (*ApploaderEntryFunc)(ApploaderInitFunc *init, ApploaderMainFunc *main,
        ApploaderCloseFunc *close);

struct ApploaderHeader {
    char revision[0x10];
    ApploaderEntryFunc entry;
    u32 size;
    u32 bs2Size;
    u8 _1c[0x20 - 0x1c];
};
size_assert(ApploaderHeader, 0x20);
