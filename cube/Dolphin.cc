// clang-format off
//
// Resources:
// - https://github.com/dolphin-emu/dolphin/blob/2503a/Source/Core/Core/IOS/DolphinDevice.cpp
//
// clang-format on

#include "Dolphin.hh"

#include <portable/Array.hh>

Dolphin::Dolphin() : IOS::Resource("/dev/dolphin", IOS::Mode::None, false) {}

Dolphin::~Dolphin() {}

bool Dolphin::ok() const {
    return Resource::ok();
}

bool Dolphin::getElapsedTime(u32 &elapsedTime) {
    alignas(0x20) IoctlvPair pairs[1];
    pairs[0].data = &elapsedTime;
    pairs[0].size = sizeof(elapsedTime);

    return ioctlv(Ioctlv::GetElapsedTime, 0, 1, pairs) == 0;
}

bool Dolphin::getVersion(Array<char, 64> &versionString, DolphinVersion &version) {
    alignas(0x20) IoctlvPair pairs[1];
    pairs[0].data = versionString.values();
    pairs[0].size = versionString.count();

    if (ioctlv(Ioctlv::GetVersion, 0, 1, pairs) != 0) {
        return false;
    }

    return DolphinVersion::Read(versionString, version);
}
