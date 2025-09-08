#include <cube/EXI.hh>

#include <cube/Memory.hh>
extern "C" {
#include <dolphin/EXIBios.h>
#include <dolphin/OSThread.h>
}
#include <payload/Lock.hh>
#include <portable/Align.hh>

struct EXIChannel {
    u32 cpr;
    u32 mar;
    u32 length;
    u32 cr;
    u32 data;
};
size_assert(EXIChannel, 0x14);

extern "C" volatile EXIChannel exi[3];

bool EXI::Device::acquire(u32 channel, u32 device, u32 frequency, bool *wasDetached) {
    Lock<NoInterrupts> lock;

    m_channel = channel;
    m_ok = false;

    while (!EXILock(channel, device, HandleUnlock)) {
        OSSleepThread(&s_queues[channel]);
    }

    if (wasDetached && *wasDetached) {
        EXIUnlock(channel);
        return false;
    }

    if (!EXISelect(channel, device, frequency)) {
        EXIUnlock(channel);
        return false;
    }

    m_ok = true;
    return true;
}

void EXI::Device::release() {
    if (m_ok) {
        EXIDeselect(m_channel);
        EXIUnlock(m_channel);
        m_ok = false;
    }
}

void EXI::Device::release2() {
    if (m_ok) {
        EXIDeselect(m_channel);
        exi[m_channel].cpr = (exi[m_channel].cpr & 0x405) | 5 << 4;
        u32 chunk = ~0;
        u32 chunkSize = 1;
        exi[m_channel].data = chunk;
        exi[m_channel].cr = (chunkSize - 1) << 4 | 0 << 2 | 1 << 0;
        while (exi[m_channel].cr & 1) {}
        exi[m_channel].cpr = exi[m_channel].cpr & 0x405;
        EXIUnlock(m_channel);
        m_ok = false;
    }
}

bool EXI::Device::immRead(void *buffer, u32 size) {
    return EXIImmEx(m_channel, buffer, size, EXI_READ);
}

bool EXI::Device::immWrite(const void *buffer, u32 size) {
    return EXIImmEx(m_channel, const_cast<void *>(buffer), size, EXI_WRITE);
}

bool EXI::Device::dmaRead(void *buffer, u32 size) {
    if (Memory::IsMEM1(buffer) && Memory::IsAligned(buffer, 0x20)) {
        u32 alignedSize = AlignDown<u32>(size, 0x20);
        if (alignedSize != 0) {
            if (!EXIDma(m_channel, buffer, alignedSize, EXI_READ, nullptr)) {
                return false;
            }
            if (!EXISync(m_channel)) {
                return false;
            }
        }

        buffer = reinterpret_cast<u8 *>(buffer) + alignedSize;
        size -= alignedSize;
    }
    return immRead(buffer, size);
}

bool EXI::Device::dmaWrite(const void *buffer, u32 size) {
    if (Memory::IsMEM1(buffer) && Memory::IsAligned(buffer, 0x20)) {
        u32 alignedSize = AlignDown<u32>(size, 0x20);
        if (alignedSize != 0) {
            if (!EXIDma(m_channel, const_cast<void *>(buffer), alignedSize, EXI_WRITE, nullptr)) {
                return false;
            }
            if (!EXISync(m_channel)) {
                return false;
            }
        }

        buffer = reinterpret_cast<const u8 *>(buffer) + alignedSize;
        size -= alignedSize;
    }
    return immWrite(buffer, size);
}

bool EXI::GetID(u32 channel, u32 device, u32 &id) {
    return EXIGetID(channel, device, &id);
}

void EXI::HandleUnlock(s32 channel, OSContext * /* context */) {
    OSWakeupThread(&s_queues[channel]);
}

Array<OSThreadQueue, 3> EXI::s_queues;
