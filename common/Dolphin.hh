#pragma once

#include "common/ios/Resource.hh"

class Dolphin : private IOS::Resource {
public:
    struct Version {
        u32 major;
        u32 minor;
        u32 patch;
    };

    Dolphin();
    ~Dolphin();
    bool ok();

    bool getVersion(Version &version);
    bool getRandom(void *buffer, u32 size);

private:
    class Ioctlv {
    public:
        enum {
            GetVersion = 0x2,
            GetRandom = 0xb,
        };

    private:
        Ioctlv();
    };
};

bool operator==(const Dolphin::Version &a, const Dolphin::Version &b);
bool operator!=(const Dolphin::Version &a, const Dolphin::Version &b);
bool operator<(const Dolphin::Version &a, const Dolphin::Version &b);
bool operator>(const Dolphin::Version &a, const Dolphin::Version &b);
bool operator<=(const Dolphin::Version &a, const Dolphin::Version &b);
bool operator>=(const Dolphin::Version &a, const Dolphin::Version &b);
