#include "System.hh"

#include "game/AppMgr.hh"
#include "game/CardMgr.hh"
#include "game/ErrorViewApp.hh"
#include "game/GameAudioMain.hh"
#include "game/GameClock.hh"
#include "game/MoviePlayer.hh"
#include "game/NetGameMgr.hh"
#include "game/PadMgr.hh"
#include "game/ResMgr.hh"
#include "game/SequenceApp.hh"

#include <cube/Platform.hh>
#include <cube/SCFile.hh>
extern "C" {
#include <dolphin/DVD.h>
}
#include <jsystem/J2DPane.hh>
#include <jsystem/J3DSys.hh>
#include <payload/CourseManager.hh>
#include <payload/DOLBinary.hh>
#include <payload/PayloadBinary.hh>
#include <payload/online/CubeServerManager.hh>
#include <portable/Log.hh>

void System::Init() {
    REPLACED(Init)();

    if (!Platform::IsGameCube()) {
        SCFile sc;
        u8 ar;
        if (sc.get("IPL.AR", ar) && ar) {
            s_defaultAspectRatio = 38.0f / 21.0f;
        }
    }

    DEBUG("%p %p mainI.dol", DOLBinary::Start(), DOLBinary::TextSectionEnd());
    DEBUG("%p %p payloadI.bin", PayloadBinary::Start(), PayloadBinary::TextSectionEnd());
    const JKRHeap *courseHeap = ResMgr::GetCourseHeap();
    DEBUG("%p %p CourseHeap", courseHeap->getStartAddr(), courseHeap->getEndAddr());
    const u8 *courseManagerStart = reinterpret_cast<u8 *>(CourseManager::Instance());
    const u8 *courseManagerEnd = courseManagerStart + sizeof(CourseManager);
    DEBUG("%p %p CourseManager", courseManagerStart, courseManagerEnd);
    DEBUG("%p %p mainD.dol", DOLBinary::CtorsSectionStart(), DOLBinary::End());
    DEBUG("%p %p payloadD.bin", PayloadBinary::CtorsSectionStart(), PayloadBinary::End());
    const u8 *serverManagerStart = reinterpret_cast<u8 *>(CubeServerManager::Instance());
    const u8 *serverManagerEnd = serverManagerStart + sizeof(ServerManager);
    DEBUG("%p %p ServerManager", serverManagerStart, serverManagerEnd);
    const JKRHeap *systemHeap = JKRHeap::GetSystemHeap();
    DEBUG("%p %p SystemHeap", systemHeap->getStartAddr(), systemHeap->getEndAddr());
    DEBUG("%p %p AppHeap", s_appHeap->getStartAddr(), s_appHeap->getEndAddr());
}

void System::Run() {
    while (true) {
        CardMgr::Probe();
        s_dvdState = DVDGetDriveStatus();
        switch (s_dvdState) {
        case DVD_STATE_NO_DISK:
            ErrorViewApp::Call(2);
            break;
        case DVD_STATE_COVER_OPEN:
            ErrorViewApp::Call(1);
            break;
        case DVD_STATE_WRONG_DISK:
            ErrorViewApp::Call(3);
            break;
        case DVD_STATE_RETRY:
            ErrorViewApp::Call(4);
            break;
        case DVD_STATE_FATAL_ERROR:
            ErrorViewApp::Call(5);
            break;
        }

        if (GameAudio::Main::Instance()->isActive()) {
            GameAudio::Main::Instance()->framework();
        }

        s_display->beginRender();

        NetGameMgr *netGameMgr = NetGameMgr::Instance();
        bool isNetGameActive = netGameMgr->isActive();
        netGameMgr->adjustFrame();
        GameClock::Move();
        J3DSys::Instance().drawInit();
        if (isNetGameActive) {
            PadMgr::Framework();
        }
        netGameMgr->framework();
        AppMgr::Draw();

        if (!isNetGameActive) {
            PadMgr::Framework();
        }
        AppMgr::Calc();

        s_display->endRender();
        s_display->endFrame();

        if (MoviePlayer::Instance()) {
            MoviePlayer::DrawDone();
        }
    }
}

void System::StartAudio() {
    SequenceApp::Create();
    s_loadTask->request(REPLACED(StartAudio), nullptr, nullptr);
}

JFWDisplay *System::GetDisplay() {
    return s_display;
}

JKRHeap *System::GetAppHeap() {
    return s_appHeap;
}

JKRTask *System::GetLoadTask() {
    return s_loadTask;
}

void System::SetAspectRatio(u32 index) {
    f32 aspectRatios[AspectRatio::Count];
    aspectRatios[0] = s_defaultAspectRatio;
    aspectRatios[1] = 4.0f / 3.0f;
    aspectRatios[2] = 19.0f / 14.0f;
    aspectRatios[3] = 3.0f / 2.0f;
    aspectRatios[4] = 16.0f / 10.0f;
    aspectRatios[5] = 16.0f / 9.0f;
    aspectRatios[6] = 38.0f / 21.0f;
    aspectRatios[7] = 21.0f / 9.0f;
    f32 aspectRatio = aspectRatios[index];

    u32 width = aspectRatio / (19.0f / 14.0f) * 608.0f;
    s32 left = 0 + (608 - static_cast<s32>(width)) / 2;
    s32 right = 608 + (static_cast<s32>(width) - 608) / 2;
    s_j2dOrtho->m_orthoBox.start.x = left;
    s_j2dOrtho->m_orthoBox.end.x = right;

    s_aspect = aspectRatio;
    s_aspectDiv2 = aspectRatio * 448.0f / (0.5f * 448.0f - 1.0f);
    s_aspectSub = aspectRatio * 149.0f / 112.0f;

    J2DPane::SetAspectRatio(aspectRatio);
}

void System::StartAudio(void * /* userData */) {}

f32 System::s_defaultAspectRatio = 19.0f / 14.0f;
