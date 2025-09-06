#include "LogoApp.hh"

#include "game/BBAMgr.hh"
#include "game/CardAgent.hh"
#include "game/GameAudioMain.hh"
#include "game/ResMgr.hh"
#include "game/SequenceApp.hh"
#include "game/System.hh"

#include <payload/CourseManager.hh>
#include <payload/network/CubeDNS.hh>
#include <payload/network/CubeNetwork.hh>
#include <payload/online/Client.hh>
#include <payload/online/CubeServerManager.hh>
#include <portable/Log.hh>

void LogoApp::draw() {}

void LogoApp::calc() {
    switch (m_state) {
    case 0:
        ResMgr::LoadKeepData();
        break;
    case 1:
        if (!ResMgr::IsFinishedLoadingArc(ResMgr::ArchiveID::MRAMLoc)) {
            return;
        }
        CardAgent::Ask(CardAgent::Command::CheckSystemFile, 0);
        System::StartAudio();
        INFO("Loading bgm_0.aw...");
        break;
    case 2:
        if (!GameAudio::Main::Instance()->isWaveLoaded(5)) {
            return;
        }
        INFO("Loaded bgm_0.aw.");
        INFO("Loading se00_0.aw...");
        break;
    case 3:
        if (!GameAudio::Main::Instance()->isWaveLoaded(1)) {
            return;
        }
        INFO("Loaded se00_0.aw.");
        break;
    case 4:
        INFO("12345");
        return;
        {
            SOConfig &config = BBAMgr::Config();
            config.flag = 1 << 0; 
            CubeNetwork::Instance().ensureStarted(config);
        }
        break;
    case 5:
        if (!CubeNetwork::Instance().isRunning()) {
            return;
        }
        break;
    case 6:
        {
            static u32 address = 0;
            u32 a = address;
            if (CubeDNS::Instance()->resolve("google.com", address)) {
                if (address != a) {
                    DEBUG("%08x", address);
                }
            }
        }
        return;
    case 7:
        CourseManager::Instance()->start();
        CubeServerManager::Instance()->start();
        Client::Init(System::GetAppHeap(), BBAMgr::Config());
        SequenceApp::Call(SceneType::Title);
        return;
    }
    m_state++;
}

LogoApp *LogoApp::Create() {
    s_instance = new (System::GetAppHeap(), 0x4) LogoApp;
    s_instance->m_state = 0;
    return s_instance;
}
