#pragma once

#include "cube/ios/Resource.hh"

#include <portable/DolphinVersion.hh>

class Dolphin : private IOS::Resource {
public:
    Dolphin();
    ~Dolphin();
    bool ok() const;

    bool getElapsedTime(u32 &elapsedTime);
    bool getVersion(Array<char, 64> &versionString, DolphinVersion &version);

private:
    class Ioctlv {
    public:
        enum {
            GetElapsedTime = 0x1,
            GetVersion = 0x2,
        };

    private:
        Ioctlv();
    };
};
