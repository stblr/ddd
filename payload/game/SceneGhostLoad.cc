#include "SceneGhostLoad.hh"

#include "game/AppMgr.hh"
#include "game/GameAudioMain.hh"
#include "game/GhostAction.hh"
#include "game/MenuBackground.hh"
#include "game/MenuTitleLine.hh"
#include "game/RaceApp.hh"
#include "game/SceneGhostCheckSave.hh"
#include "game/SequenceApp.hh"
#include "game/SequenceInfo.hh"
#include "game/System.hh"

SceneGhostLoad::SceneGhostLoad(JKRArchive *archive, JKRHeap *heap)
    : Scene(archive, heap), m_selectSlot(SceneGhostCheckSave::Instance()->selectSlot()) {}

SceneGhostLoad::~SceneGhostLoad() {}

void SceneGhostLoad::init() {
    wait();
}

void SceneGhostLoad::draw() {
    m_graphContext->setViewport();

    MenuBackground::Instance()->draw(m_graphContext);
    MenuTitleLine::Instance()->draw(m_graphContext);

    m_selectSlot.draw(m_graphContext);
}

void SceneGhostLoad::calc() {
    m_selectSlot.processCards();

    (this->*m_state)();

    MenuBackground::Instance()->calc();
    MenuTitleLine::Instance()->calc();

    m_selectSlot.calcAnm();
}

void SceneGhostLoad::wait() {
    m_state = &SceneGhostLoad::stateWait;
}

void SceneGhostLoad::slideIn() {
    MenuTitleLine::Instance()->drop(MenuTitleLine::Title::LoadGhostData);
    m_selectSlot.frameIn();
    m_state = &SceneGhostLoad::stateSlideIn;
}

void SceneGhostLoad::slideOut() {
    MenuTitleLine::Instance()->lift();
    m_selectSlot.frameOut();
    m_state = &SceneGhostLoad::stateSlideOut;
}

void SceneGhostLoad::idle() {
    m_state = &SceneGhostLoad::stateIdle;
}

void SceneGhostLoad::nextScene() {
    m_state = &SceneGhostLoad::stateNextScene;
}

void SceneGhostLoad::nextRace() {
    m_state = &SceneGhostLoad::stateNextRace;
}

void SceneGhostLoad::stateWait() {
    if (m_selectSlot.isWaiting()) {
        return;
    }

    if (m_selectSlot.canLoad()) {
        m_selectSlot.initLoad();
        slideIn();
    } else {
        GameAudio::Main::Instance()->fadeOutAll(15);
        System::GetDisplay()->startFadeOut(15);
        nextRace();
    }
}

void SceneGhostLoad::stateSlideIn() {
    if (m_selectSlot.isIdle()) {
        idle();
    }
}

void SceneGhostLoad::stateSlideOut() {
    if (m_nextScene == SceneType::None) {
        nextRace();
    } else {
        nextScene();
    }
}

void SceneGhostLoad::stateIdle() {
    u32 action = m_selectSlot.selectSlot();
    switch (action) {
    case SelectSlot::Action::Next:
        m_selectSlot.frameOut();
        break;
    case SelectSlot::Action::Prev:
        m_nextScene = SceneType::CourseSelect;
        slideOut();
        break;
    case SelectSlot::Action::Skip:
        m_nextScene = SceneType::None;
        slideOut();
        break;
    }
}

void SceneGhostLoad::stateNextScene() {
    if (m_selectSlot.isWaiting()) {
        return;
    }

    if (!SequenceApp::Instance()->ready(m_nextScene)) {
        return;
    }

    SequenceApp::Instance()->setNextScene(m_nextScene);
}

void SceneGhostLoad::stateNextRace() {
    if (m_selectSlot.isWaiting()) {
        return;
    }

    if (!SequenceApp::Instance()->checkFinishAllLoading()) {
        return;
    }

    AppMgr::Request(AppMgr::Request::DestroyApp);
    RaceApp::Call();
    SequenceInfo::Instance().m_ghostAction = GhostAction::None;
}
