#include <common/ios/Resource.hh>

#include <common/DCache.hh>
#include <common/Memory.hh>

extern "C" {
#include <string.h>
}

namespace IOS {

Resource::Resource(s32 fd) : m_fd(fd) {}

Resource::Resource(const char *path, bool read, bool write) : m_fd(-1) {
    open(path, read, write);
}

Resource::~Resource() {
    if (m_fd >= 0) {
        close();
    }
}

s32 Resource::ioctl(u32 ioctl, const void *input, u32 inputSize, void *output, u32 outputSize) {
    DCache::Flush(input, inputSize);
    DCache::Flush(output, outputSize);

    alignas(0x20) Request request;
    memset(&request, 0, sizeof(request));
    request.command = Command::Ioctl;
    request.fd = m_fd;
    request.ioctl.ioctl = ioctl;
    request.ioctl.input = Memory::VirtualToPhysical(input);
    request.ioctl.inputSize = inputSize;
    request.ioctl.output = Memory::VirtualToPhysical(output);
    request.ioctl.outputSize = outputSize;

    Sync(request);

    DCache::Invalidate(output, outputSize);

    return request.result;
}

s32 Resource::ioctlv(u32 ioctlv, u32 inputCount, u32 outputCount, IoctlvPair *pairs) {
    for (u32 i = 0; i < inputCount + outputCount; i++) {
        if (pairs[i].data && pairs[i].size != 0) {
            DCache::Flush(pairs[i].data, pairs[i].size);
            pairs[i].data = reinterpret_cast<void *>(Memory::VirtualToPhysical(pairs[i].data));
        }
    }
    DCache::Flush(pairs, (inputCount + outputCount) * sizeof(IoctlvPair));

    alignas(0x20) Request request;
    memset(&request, 0, sizeof(request));
    request.command = Command::Ioctlv;
    request.fd = m_fd;
    request.ioctlv.ioctlv = ioctlv;
    request.ioctlv.inputCount = inputCount;
    request.ioctlv.outputCount = outputCount;
    request.ioctlv.pairs = Memory::VirtualToPhysical(pairs);

    Sync(request);

    for (u32 i = inputCount; i < inputCount + outputCount; i++) {
        if (pairs[i].data && pairs[i].size != 0) {
            pairs[i].data = Memory::PhysicalToVirtual<void *>(reinterpret_cast<u32>(pairs[i].data));
            DCache::Invalidate(pairs[i].data, pairs[i].size);
        }
    }

    return request.result;
}

bool Resource::ok() const {
    return m_fd >= 0;
}

s32 Resource::open(const char *path, bool read, bool write) {
    alignas(0x20) char alignedPath[0x40];
    strlcpy(alignedPath, path, sizeof(alignedPath));

    DCache::Flush(alignedPath, sizeof(alignedPath));

    alignas(0x20) Request request;
    memset(&request, 0, sizeof(request));
    request.command = Command::Open;
    request.open.path = Memory::VirtualToPhysical(alignedPath);
    request.open.mode = write << 1 | read << 0;

    Sync(request);

    m_fd = request.result;
    return request.result;
}

s32 Resource::close() {
    alignas(0x20) Request request;
    memset(&request, 0, sizeof(request));
    request.command = Command::Close;
    request.fd = m_fd;

    Sync(request);

    m_fd = -1;
    return request.result;
}

} // namespace IOS
