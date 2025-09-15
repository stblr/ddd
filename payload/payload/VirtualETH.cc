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

#include "payload/Lock.hh"

#include <cube/Arena.hh>
#include <cube/Clock.hh>
#include <cube/ECID.hh>
#include <cube/EXI.hh>
extern "C" {
#include <dolphin/EXIBios.h>
#include <dolphin/OSInterrupt.h>
}
#include <portable/Bytes.hh>
#include <portable/Log.hh>

extern "C" {
#include <string.h>
}

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

void VirtualETH::getMACAddr(u8 macaddr[6]) {
    memcpy(macaddr, m_macAddr, 6);
}

BOOL VirtualETH::getLinkStateAsync(BOOL *status) {
    Lock<NoInterrupts> lock;
    *status = m_latchingLinkStatus;
    m_latchingLinkStatus = m_linkStatus;
    return true;
}

void VirtualETH::Init() {
    s_instance = new (MEM1Arena::Instance(), 0x4) VirtualETH;
}

VirtualETH *VirtualETH::Instance() {
    return s_instance;
}

VirtualETH::VirtualETH() {
    OSInitMessageQueue(&m_queue, m_messages.values(), m_messages.count());
    void *param = this;
    OSCreateThread(&m_thread, Run, param, m_stack.values() + m_stack.count(), m_stack.count(), 2,
            0);
    OSResumeThread(&m_thread);
}

void *VirtualETH::run() {
    DEBUG("run");
    while (true) {
        OSReceiveMessage(&m_queue, nullptr, OS_MESSAGE_BLOCK);
        DEBUG("irq");

        u8 eir;
        if (!readControlRegister(EIR, eir, false)) {
            continue;
        }
        DEBUG("EIR: %02x", eir);

        u8 eirMask = 0;

        if (eir & 1 << 4 /* LINKIF */) {
            u16 phir;
            if (readPHYRegister(PHIR, phir)) {
                DEBUG("PHIR: %04x", phir);
            }

            u16 phstat1, phstat2;
            if (readPHYRegister(PHSTAT1, phstat1) && readPHYRegister(PHSTAT2, phstat2)) {
                DEBUG("PHSTAT1: %04x", phstat1);
                DEBUG("PHSTAT2: %04x", phstat2);

                Lock<NoInterrupts> lock;
                if (!(phstat1 & 1 << 2 /* LLSTAT */)) {
                    m_latchingLinkStatus = false;
                }
                m_linkStatus = phstat2 & 1 << 10;
            }

            eir &= ~(1 << 4); // LINKIF
        }

        bitFieldClear(EIR, eirMask);
    }
}

void VirtualETH::handleEXT() {
    m_wasDetached = true;
}

void VirtualETH::handleEXI() {
    OSSendMessage(&m_queue, nullptr, OS_MESSAGE_NOBLOCK);
}

bool VirtualETH::init() {
    m_bank = 0;
    m_latchingLinkStatus = false;
    m_linkStatus = false;

    // TODO is this useful?
    u32 id;
    if (!EXI::GetID(m_channel, 0, id)) {
        return false;
    }
    DEBUG("%08x", id);

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
            DEBUG("Failed to read ESTAT");
            return false;
        }

        if (estat & 1 << 0) {
            break;
        }
    } while (Clock::GetMonotonicTicks() < start + Clock::MicrosecondsToTicks(300));
    if (!(estat & 1 << 0)) {
        DEBUG("Unexpected ESTAT %02x", estat);
        return false;
    }

    u16 phid1;
    if (!readPHYRegister(PHID1, phid1)) {
        DEBUG("Failed to read PHID1");
        return false;
    }
    u16 phid2;
    if (!readPHYRegister(PHID2, phid2)) {
        DEBUG("Failed to read PHID2");
        return false;
    }
    u32 phid = phid1 << 16 | phid2 << 0;
    if (phid != 0x00831400) {
        DEBUG("Unexpected PHID %08x", phid);
        return false;
    }

#if 0
#define DUMP_REG(reg) \
    { \
        u16 r; \
        if (!readControlRegister(reg, r, false)) { \
            return false; \
        } \
        DEBUG(#reg ": %04x", r); \
    }

    DUMP_REG(ERDPT)   // 1530 -> 0
    DUMP_REG(EWRPT)   // 0 -> 4096
    DUMP_REG(ETXST)   // 0 -> 4096
    DUMP_REG(ETXND)   // 0 -> 8191
    DUMP_REG(ERXST)   // 1530 -> 0
    DUMP_REG(ERXND)   // 8191 -> 4095
    DUMP_REG(ERXRDPT) // 1530 -> 4095
    DUMP_REG(ERXWRPT) // 0
#endif

    if (!initFilters()) {
        DEBUG("Failed to initialize filters");
        return false;
    }

    if (!initMAC()) {
        DEBUG("Failed to initialize MAC");
        return false;
    }

    if (!initMACAddr()) {
        DEBUG("Failed to initialize MAC address");
        return false;
    }
    const u8 *m = m_macAddr;
    DEBUG("%02x:%02x:%02x:%02x:%02x:%02x", m[0], m[1], m[2], m[3], m[4], m[5]);

    if (!initPHY()) {
        DEBUG("Failed to initialize PHY");
        return false;
    }

    if (!initInterrupts()) {
        DEBUG("Failed to initialize interrupts");
        return false;
    }

    if (m_channel == 2) {
        OSSetInterruptHandler(25, HandleEXI);
    } else {
        EXISetExiCallback(m_channel + m_device, HandleEXI);
    }
    return true;
}

bool VirtualETH::initFilters() {
    u8 erxfcon = 0;
    erxfcon |= 1 << 7; // UCEN
    erxfcon |= 1 << 5; // CRCEN
    erxfcon |= 1 << 1; // MCEN
    erxfcon |= 1 << 0; // BCEN
    return writeControlRegister(ERXFCON, erxfcon);
}

bool VirtualETH::initMAC() {
    bool result = true;

    u8 macon1 = 0;
    macon1 |= 1 << 0; // MARXEN
    result = result && writeControlRegister(MACON1, macon1);

    u8 macon3 = 0;
    macon3 |= 1 << 5; // PADCFG
    macon3 |= 1 << 4; // TXCRCEN
    macon3 |= 1 << 1; // FRMLNEN
    result = result && writeControlRegister(MACON3, macon3);

    u8 macon4 = 0;
    macon4 |= 1 << 6; // DEFER
    result = result && writeControlRegister(MACON4, macon4);

    u8 mabbipg = 0x12;
    result = result && writeControlRegister(MABBIPG, mabbipg);

    u16 maipg = 0x0c12;
    result = result && writeControlRegister(MAIPG, maipg);

    return result;
}

bool VirtualETH::initMACAddr() {
    Array<u32, 4> ecid = ECID::Get();
    Array<u8, 19> data;
    for (u32 i = 0; i < ecid.count(); i++) {
        Bytes::WriteBE<u32>(data.values(), i * 4, ecid[i]);
    }
    data[16] = 0x04;
    data[17] = 0xa3;
    data[18] = 0x00;

    u32 sum = m_channel;
    for (u32 i = 0; i < 6; i++) {
        sum += Bytes::ReadBE<u32>(data.values(), i * 3) >> 8;
        sum = (sum & 0xffffff) + (sum >> 24);
    }

    m_macAddr[0] = 0x00;
    m_macAddr[1] = 0x09;
    m_macAddr[2] = 0xbf;
    m_macAddr[3] = sum >> 16;
    m_macAddr[4] = sum >> 8;
    m_macAddr[5] = sum >> 0;

    bool result = true;
    result = result && writeControlRegister(MAADR1, m_macAddr[0]);
    result = result && writeControlRegister(MAADR2, m_macAddr[1]);
    result = result && writeControlRegister(MAADR3, m_macAddr[2]);
    result = result && writeControlRegister(MAADR4, m_macAddr[3]);
    result = result && writeControlRegister(MAADR5, m_macAddr[4]);
    result = result && writeControlRegister(MAADR6, m_macAddr[5]);
    return result;
}

bool VirtualETH::initPHY() {
    bool result = true;

    u16 phcon2 = 0;
    phcon2 |= 1 << 8; // HDLDIS
    result = result && writePHYRegister(PHCON2, phcon2);

    u16 phlcon = 0;
    phlcon |= 1 << 13; // Reserved
    phlcon |= 1 << 12; // Reserved
    phlcon |= 4 << 8;  // LACFG
    phlcon |= 7 << 4;  // LBCFG
    phlcon |= 1 << 2;  // LFRQ
    phlcon |= 1 << 1;  // STRCH
    result = result && writePHYRegister(PHLCON, phlcon);

    u16 phie = 0;
    phie |= 1 << 4; // PLNKIE
    phie |= 1 << 1; // PGEIE
    result = result && writePHYRegister(PHIE, phie);

    return result;
}

bool VirtualETH::initInterrupts() {
    u8 eieMask = 0;
    eieMask |= 1 << 7; // INTIE
    eieMask |= 1 << 4; // LINKIE
    return bitFieldSet(EIE, eieMask);
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

void *VirtualETH::Run(void *param) {
    return static_cast<VirtualETH *>(param)->run();
}

void VirtualETH::HandleEXT(s32 /* chan */, OSContext * /* context */) {
    s_instance->handleEXT();
}

void VirtualETH::HandleEXI(s32 /* chan */, OSContext * /* context */) {
    s_instance->handleEXI();
}

void VirtualETH::HandleEXI(s16 /* interrupt */, OSContext * /* context */) {
    s_instance->handleEXI();
}

VirtualETH *VirtualETH::s_instance = nullptr;
