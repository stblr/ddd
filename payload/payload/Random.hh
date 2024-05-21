#pragma once

#include <common/Array.hh>

class Random {
public:
    static void Init();
    static bool IsSecure();
    static void Get(void *data, size_t size);

private:
    static bool InitWithDolphinDevice();
    static bool InitWithDiscTimings();

    static bool s_isInit;
    static bool s_isSecure;
    alignas(0x20) static Array<u8, 32 + 256> s_buffer;
    static u16 s_offset;
};
