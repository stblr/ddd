#include "FS.hh"

#include "common/Array.hh"
#include "common/ios/File.hh"

extern "C" {
#include <stdio.h>
}

FS::FS() : IOS::Resource("/dev/fs", IOS::Mode::None) {}

FS::~FS() {}

bool FS::ok() const {
    return Resource::ok();
}

bool FS::writeFile(const char *path, const void *src, u32 size, u16 mode, u8 attributes) {
    remove(path);
    createFile(path, mode, attributes);

    IOS::File file(path, IOS::Mode::Write);
    if (!file.ok()) {
        return false;
    }

    return file.write(src, size) == static_cast<s32>(size);
}

bool FS::createDir(const char *path, u16 mode, u8 attributes) {
    return create(Ioctl::CreateDir, path, mode, attributes);
}

bool FS::remove(const char *path) {
    alignas(0x20) Array<char, 0x40> in(0x00);

    if (snprintf(in.values(), in.count(), "%s", path) >= static_cast<s32>(in.count())) {
        return false;
    }

    return ioctl(Ioctl::Remove, in.values(), in.count(), nullptr, 0) == 0;
}

bool FS::rename(const char *srcPath, const char *dstPath) {
    alignas(0x20) Array<char, 0x80> in(0x00);

    if (snprintf(in.values() + 0x00, 0x40, "%s", srcPath) >= 0x40) {
        return false;
    }
    if (snprintf(in.values() + 0x40, 0x40, "%s", dstPath) >= 0x40) {
        return false;
    }

    return ioctl(Ioctl::Rename, in.values(), in.count(), nullptr, 0) == 0;
}

bool FS::createFile(const char *path, u16 mode, u8 attributes) {
    return create(Ioctl::CreateFile, path, mode, attributes);
}

bool FS::create(u32 ioctl, const char *path, u16 mode, u8 attributes) {
    alignas(0x20) Array<u8, 0x4c> in(0x00);

    if (snprintf(reinterpret_cast<char *>(in.values() + 0x6), 0x40, "%s", path) >= 0x40) {
        return false;
    }
    for (u32 i = 0; i < 3; i++) {
        in[0x46 + i] = mode >> (2 - i) * 3 & 3;
    }
    in[0x49] = attributes;

    return FS::ioctl(ioctl, in.values(), in.count(), nullptr, 0) == 0;
}
