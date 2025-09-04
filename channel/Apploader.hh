#pragma once

#include <cube/ApploaderHeader.hh>
#include <portable/Array.hh>

class Apploader {
public:
    typedef bool (*ReadFunc)(void *dst, u32 size, u32 offset);

    static GameEntryFunc Run(ReadFunc read);

private:
    Apploader();

    static bool Read(ReadFunc read, void *dst, u32 size, u32 offset, const Array<u8, 32> *&hashes);
    static void Report(const char *format, ...);

    static const Array<u8, 32> HashesP[18];
    static const Array<u8, 32> HashesE[18];
    static const Array<u8, 32> HashesJ[18];
};
