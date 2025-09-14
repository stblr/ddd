#include "SceneCardSelect.hh"

#include "game/AppMgr.hh"
#include "game/GhostAction.hh"
#include "game/MenuBackground.hh"
#include "game/MenuTitleLine.hh"
#include "game/RaceApp.hh"
#include "game/SequenceApp.hh"
#include "game/SequenceInfo.hh"

SceneCardSelect::SceneCardSelect(JKRArchive *archive, JKRHeap *heap) : Scene(archive, heap) {
    m_screen.set("SelectMemoryCard.blo", 0x20000, m_archive);
}

SceneCardSelect::~SceneCardSelect() {}

void SceneCardSelect::init() {
    wait();
}

void SceneCardSelect::draw() {
    m_graphContext->setViewport();

    MenuBackground::Instance()->draw(m_graphContext);
    MenuTitleLine::Instance()->draw(m_graphContext);

    m_screen.draw(0.0f, 0.0f, m_graphContext);
}

void SceneCardSelect::calc() {
    (this->*m_state)();

    MenuBackground::Instance()->calc();
    MenuTitleLine::Instance()->calc();

    m_screen.animation();
}

void SceneCardSelect::wait() {
    m_state = &SceneCardSelect::stateWait;
}

void SceneCardSelect::slideIn() {
    MenuTitleLine::Instance()->drop(MenuTitleLine::Title::LoadGhostData);
    m_state = &SceneCardSelect::stateSlideIn;
}

void SceneCardSelect::slideOut() {
    MenuTitleLine::Instance()->lift();
    m_state = &SceneCardSelect::stateSlideOut;
}

void SceneCardSelect::idle() {
    m_state = &SceneCardSelect::stateIdle;
}

void SceneCardSelect::nextScene() {
    m_state = &SceneCardSelect::stateNextScene;
}

void SceneCardSelect::nextRace() {
    m_state = &SceneCardSelect::stateNextRace;
}

void SceneCardSelect::stateWait() {
    slideIn();
}

void SceneCardSelect::stateSlideIn() {
    idle();
}

void SceneCardSelect::stateSlideOut() {
    if (m_nextScene == SceneType::None) {
        nextRace();
    } else {
        nextScene();
    }
}

void SceneCardSelect::stateIdle() {}

void SceneCardSelect::stateNextScene() {
    if (!SequenceApp::Instance()->ready(m_nextScene)) {
        return;
    }

    SequenceApp::Instance()->setNextScene(m_nextScene);
}

void SceneCardSelect::stateNextRace() {
    if (!SequenceApp::Instance()->checkFinishAllLoading()) {
        return;
    }

    AppMgr::Request(AppMgr::Request::DestroyApp);
    RaceApp::Call();
    SequenceInfo::Instance().m_ghostAction = GhostAction::None;
}
