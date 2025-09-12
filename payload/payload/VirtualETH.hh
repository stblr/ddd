#pragma once

extern "C" {
#include <dolphin/ETH.h>
#include <dolphin/OSContext.h>
}

class VirtualETH {
public:
    s32 init(s32 mode);
    void getMACAddr(u8 macaddr[6]);
    void setRecvCallback(ETHCallback0 callback0, ETHCallback1 callback1);
    BOOL getLinkStateAsync(BOOL *status);
    void setProtoType(u16 *array, s32 num);
    void sendAsync(void *addr, s32 length, ETHCallback2 callback2);
    void addMulticastAddress(const u8 macaddr[6]);
    void clearMulticastAddresses();

    static void Init();
    static VirtualETH *Instance();

private:
    enum {
        ERDPT = 0x00,   // Read Pointer
        EWRPT = 0x02,   // Write Pointer
        ETXST = 0x04,   // TX Start
        ETXND = 0x06,   // TX End
        ERXST = 0x08,   // RX Start
        ERXND = 0x0a,   // RX End
        ERXRDPT = 0x0c, // RX RD Pointer
        ERXWRPT = 0x0e, // RX WR Pointer
        EIE = 0x1b,
        EIR = 0x1c,
        ESTAT = 0x1d,
        ECON2 = 0x1e,
        ECON1 = 0x1f,
        ERXFCON = 0x38,
        MACON1 = 0x40,
        MACON3 = 0x42,
        MACON4 = 0x43,
        MABBIPG = 0x44,
        MAIPG = 0x46,
        MICMD = 0x52,
        MIREGADR = 0x54,
        MIWR = 0x56,
        MIRD = 0x58,
        MAADR5 = 0x60,
        MAADR6 = 0x61,
        MAADR3 = 0x62,
        MAADR4 = 0x63,
        MAADR1 = 0x64,
        MAADR2 = 0x65,
        MISTAT = 0x6a,
    };

    enum {
        PHCON1 = 0x00, // PHY Control 1
        PHSTAT1 = 0x01,
        PHID1 = 0x02,  // PHY Device ID 1
        PHID2 = 0x03,  // PHY Device ID 2
        PHCON2 = 0x10, // PHY Control 2
        PHSTAT2 = 0x11,
        PHIE = 0x12,   // PHY Interrupt Enable
        PHIR = 0x13,   // PHY Interrupt Request
        PHLCON = 0x14, // PHY Module LED Control
    };

    VirtualETH();

    void handleEXT();
    bool init();
    bool initFilters();
    bool initMAC();
    bool initMACAddr();
    bool initPHY();
    bool getLinkStatus();

    void reset();
    bool bitFieldSet(u8 address, u8 bits);
    bool bitFieldClear(u8 address, u8 bits);
    bool setBank(u8 bank);
    bool readControlRegister(u8 address, u8 &data, bool hasDummy);
    bool writeControlRegister(u8 address, u8 data);
    bool readControlRegister(u8 address, u16 &data, bool hasDummy);
    bool writeControlRegister(u8 address, u16 data);
    bool readPHYRegister(u8 address, u16 &data);
    bool writePHYRegister(u8 address, u16 data);

    bool read(u8 command, void *buffer, u32 size, u32 frequency = 4);
    bool write(u8 command, const void *buffer, u32 size, u32 frequency = 4);

    static void HandleEXT(s32 chan, OSContext *context);

    u32 m_channel;
    u32 m_device;
    bool m_wasDetached;
    u8 m_bank;
    bool m_linkStatus;
    u8 m_macAddr[6];

    static VirtualETH *s_instance;
};
