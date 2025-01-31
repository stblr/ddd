// Resources:
// - https://www.microchip.com/mymicrochip/filehandler.aspx?ddocname=en023091

// Based on https://github.com/extremscorner/libogc2/blob/master/lwip/netif/enc28j60if.c
//
// Copyright (C) 2023 Extrems
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any
// damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any
// purpose, including commercial applications, and to alter it and
// redistribute it freely, subject to the following restrictions:
//
// 1.  The origin of this software must not be misrepresented; you
// must not claim that you wrote the original software. If you use
// this software in a product, an acknowledgment in the product
// documentation would be appreciated but is not required.
//
// 2.  Altered source versions must be plainly marked as such, and
// must not be misrepresented as being the original software.
//
// 3.  This notice may not be removed or altered from any source
// distribution.

#include "VirtualETH.hh"

#include <cube/Arena.hh>
#include <cube/Clock.hh>
#include <cube/EXI.hh>
extern "C" {
#include <dolphin/EXIBios.h>
}
#include <portable/Log.hh>

s32 VirtualETH::init(s32 /* mode */) {
    for (u32 i = 0; i < 4; i++) {
        m_channel = i < 2 ? 2 - i : 0;
        m_device = i == 2 ? 2 : 0;
        INFO("%u %u", m_channel, m_device);
        m_wasDetached = false;
        if (m_channel != 2 && m_device == 0) {
            if (!EXIAttach(m_channel, HandleEXT)) {
                continue;
            }
        }

        if (init()) {
            return 1;
        }

        if (m_channel != 2 && m_device == 0) {
            EXIDetach(m_channel);
        }
    }
    return -1;
}

void VirtualETH::Init() {
    s_instance = new (MEM1Arena::Instance(), 0x4) VirtualETH;
}

VirtualETH *VirtualETH::Instance() {
    return s_instance;
}

VirtualETH::VirtualETH() {}

void VirtualETH::handleEXT() {
    m_wasDetached = true;
}

bool VirtualETH::init() {
    m_bank = 0;

    u32 id;
    if (!EXI::GetID(m_channel, 0, id)) {
        return false;
    }

    reset();

    {
        EXI::Device device(m_channel, m_device, 0, &m_wasDetached);
        if (!device.ok()) {
            return false;
        }
        u16 cmd = 0x0;
        if (!device.immWrite(&cmd, sizeof(cmd))) {
            return false;
        }
        if (!device.immRead(&id, sizeof(id))) {
            return false;
        }
        INFO("%08x", id);
        if (id != 0xfa050000) {
            return false;
        }
    }

    u8 estat;
    s64 start = Clock::GetMonotonicTicks();
    do {
        if (!readControlRegister(ESTAT, estat, false)) {
            return false;
        }

        if (estat & 1 << 0) {
            break;
        }
    } while (Clock::GetMonotonicTicks() < start + Clock::MicrosecondsToTicks(300));
    if (!(estat & 1 << 0)) {
        return false;
    }
    INFO("estat ok");

    u16 phid1;
    if (!readPHYRegister(PHID1, phid1)) {
        return false;
    }
    DEBUG("phid1 %04x", phid1);
    u16 phid2;
    if (!readPHYRegister(PHID2, phid2)) {
        return false;
    }
    DEBUG("phid2 %04x", phid2);

    return false;
}

void VirtualETH::reset() {
    write(0xff, nullptr, 0, 0);
    Clock::WaitMicroseconds(50);
}

bool VirtualETH::bitFieldSet(u8 address, u8 bits) {
    return write(4 << 5 | (address & 0x1f), &bits, sizeof(bits));
}

bool VirtualETH::bitFieldClear(u8 address, u8 bits) {
    return write(5 << 5 | (address & 0x1f), &bits, sizeof(bits));
}

bool VirtualETH::setBank(u8 bank) {
    if (bank == m_bank) {
        return true;
    }

    if (!bitFieldClear(ECON1, m_bank) || !bitFieldSet(ECON1, bank)) {
        return false;
    }

    m_bank = bank;
    return true;
}

bool VirtualETH::readControlRegister(u8 address, u8 &data, bool hasDummy) {
    if (!setBank(address >> 5)) {
        return false;
    }

    u8 buffer[2];
    if (!read(0 << 5 | (address & 0x1f), buffer, 1 + hasDummy)) {
        return false;
    }

    data = buffer[hasDummy];
    return true;
}

bool VirtualETH::writeControlRegister(u8 address, u8 data) {
    if (!setBank(address >> 5)) {
        return false;
    }

    return write(2 << 5 | (address & 0x1f), &data, sizeof(data));
}

bool VirtualETH::readControlRegister(u8 address, u16 &data, bool hasDummy) {
    data = 0;
    for (u32 i = 0; i < 2; i++) {
        u8 byte;
        if (!readControlRegister(address + i, byte, hasDummy)) {
            return false;
        }
        data |= byte << (i * 8);
    }
    return true;
}

bool VirtualETH::writeControlRegister(u8 address, u16 data) {
    for (u32 i = 0; i < 2; i++) {
        u8 byte = data >> (i * 8);
        if (!writeControlRegister(address + i, byte)) {
            return false;
        }
    }
    return true;
}

bool VirtualETH::readPHYRegister(u8 address, u16 &data) {
    if (!writeControlRegister(MIREGADR, address)) {
        return false;
    }

    u8 micmd = 1 << 0;
    if (!writeControlRegister(MICMD, micmd)) {
        return false;
    }

    u8 mistat;
    s64 start = Clock::GetMonotonicTicks();
    do {
        if (!readControlRegister(MISTAT, mistat, true)) {
            return false;
        }

        if (!(mistat & 1 << 0)) {
            break;
        }
    } while (Clock::GetMonotonicTicks() < start + Clock::MicrosecondsToTicks(150));

    micmd = 0;
    if (!writeControlRegister(MICMD, micmd)) {
        return false;
    }

    if (mistat & 1 << 0) {
        return false;
    }

    return readControlRegister(MIRD, data, true);
}

bool VirtualETH::writePHYRegister(u8 address, u16 data) {
    if (!writeControlRegister(MIREGADR, address)) {
        return false;
    }

    if (!writeControlRegister(MIWR, data)) {
        return false;
    }

    s64 start = Clock::GetMonotonicTicks();
    do {
        u8 mistat;
        if (!readControlRegister(MISTAT, mistat, true)) {
            return false;
        }

        if (!(mistat & 1 << 0)) {
            return true;
        }
    } while (Clock::GetMonotonicTicks() < start + Clock::MicrosecondsToTicks(150));
    return false;
}

bool VirtualETH::read(u8 command, void *buffer, u32 size, u32 frequency) {
    EXI::Device device(m_channel, m_device, frequency, &m_wasDetached);
    if (!device.ok()) {
        return false;
    }
    if (!device.immWrite(&command, sizeof(command))) {
        return false;
    }
    return device.dmaRead(buffer, size);
}

bool VirtualETH::write(u8 command, const void *buffer, u32 size, u32 frequency) {
    EXI::Device device(m_channel, m_device, frequency, &m_wasDetached);
    if (!device.ok()) {
        return false;
    }
    if (!device.immWrite(&command, sizeof(command))) {
        return false;
    }
    return device.dmaWrite(buffer, size);
}

void VirtualETH::HandleEXT(s32 /* chan */, OSContext * /* context */) {
    s_instance->handleEXT();
}

VirtualETH *VirtualETH::s_instance = nullptr;
