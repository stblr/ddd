#pragma once

#include <cube/Dolphin.hh>
extern "C" {
#include <dolphin/OSThread.h>
}
#include <jsystem/JKRHeap.hh>
#include <portable/Array.hh>
#include <portable/Color.hh>
#include <portable/Optional.hh>

class PerfOverlay {
public:
    void beginFrame();
    void beginDraw();
    void beginRender();
    void endDraw();
    void beginCalc();
    void endCalc();
    void draw();
    void endRender();
    void handleSwitchThread(OSThread *prev, OSThread *next);

    static void Init(JKRHeap *heap);
    static PerfOverlay *Instance();

private:
    enum {
        Width = 600,
    };

    class TimeBar {
    public:
        TimeBar(PerfOverlay &perfOverlay);

        void draw(s32 y, Color color);
        void calc();
        void begin();
        void end();

    private:
        PerfOverlay &m_perfOverlay;
        s64 m_start;
        s64 m_duration;
        s32 m_x;
        s32 m_w;
        u32 m_dolphinStart;
        u32 m_dolphinDuration;
        s32 m_dolphinX;
        s32 m_dolphinW;
    };

    PerfOverlay();

    s32 convertInstantToX(s64 instant);
    s32 convertDurationToW(s64 duration);
    s32 convertDolphinInstantToX(u32 instant);
    s32 convertDolphinDurationToW(u32 duration);
    void processHeap(JKRHeap *heap);
    void fillHeaps(s32 x0, s32 x1, JKRHeap *heap);

    static s32 ConvertPtrToX(const void *ptr);
    static void FillBox(s32 x, s32 y, s32 w, s32 h, Color color);
    static void FillBoxes(s32 y, Array<Color, Width> &colors);

    s64 m_frameDuration;
    s64 m_frameStart;
    Optional<Dolphin> m_dolphin;
    u32 m_dolphinFrameStart;
    u32 m_dolphinFrameDuration;
    TimeBar m_drawBar;
    TimeBar m_calcBar;
    OSThread *m_mainThread;
    OSThread *m_prevThread;
    s32 m_prevThreadIndex;
    Array<OSThread *, Width> m_threads;
    Array<Color, Width> m_threadColors;
    TimeBar m_renderBar;
    Array<JKRHeap *, Width> m_heaps;
    Array<Color, Width> m_heapColors;

    static PerfOverlay *s_instance;
};
