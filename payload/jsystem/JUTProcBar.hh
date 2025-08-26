#pragma once

#include <portable/Types.hh>

class JUTProcBar {
public:
    void cpuStart();
    void cpuEnd();

    static JUTProcBar *Instance();

private:
    class Time {
    public:
        void start(u8 r, u8 g, u8 b);
        void end();

    private:
        u32 m_start;
        u32 m_cost;
        u8 _08[0x10 - 0x08];
        u8 m_r;
        u8 m_g;
        u8 m_b;
    };
    size_assert(Time, 0x14);

    u8 _000[0x028 - 0x000];
    Time m_cpu;
    u8 _03c[0x10c - 0x03c];

public:
    bool m_visible;

private:
    u8 _10d[0x130 - 0x10d];

public:
    bool m_heapBarVisible;

private:
    u8 _131[0x134 - 0x131];

    static JUTProcBar *s_instance;
};
size_assert(JUTProcBar, 0x134);
