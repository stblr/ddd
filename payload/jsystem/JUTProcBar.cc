#include "JUTProcBar.hh"

#include <cube/Clock.hh>
extern "C" {
#include <dolphin/OSTime.h>
}

void JUTProcBar::cpuStart() {
    m_cpu.start(255, 129, 30);
}

void JUTProcBar::cpuEnd() {
    m_cpu.end();
}

JUTProcBar *JUTProcBar::Instance() {
    return s_instance;
}

void JUTProcBar::Time::start(u8 r, u8 g, u8 b) {
    m_r = r;
    m_g = g;
    m_b = b;
    m_start = OSGetTick();
}

void JUTProcBar::Time::end() {
    m_cost = Clock::TicksToMicroseconds(OSGetTick() - m_start);
    if (m_cost == 0) {
        m_cost = 1;
    }
}
