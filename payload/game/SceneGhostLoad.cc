#include "SceneGhostLoad.hh"

#include "game/GhostAction.hh"
#include "game/MenuBackground.hh"
#include "game/MenuTitleLine.hh"
#include "game/SceneGhostCheckSave.hh"
#include "game/SequenceApp.hh"

SceneGhostLoad::SceneGhostLoad(JKRArchive *archive, JKRHeap *heap) : Scene(archive, heap) {
    m_selectSlot.setup(archive, heap);
}

SceneGhostLoad::~SceneGhostLoad() {}

void SceneGhostLoad::init() {
    SceneGhostCheckSave::Instance()->m_ghostAction = GhostAction::Load;

    m_selectSlot.init();

    slideIn();
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
    m_state = &SceneGhostLoad::stateSlideOut;
}

void SceneGhostLoad::idle() {
    m_state = &SceneGhostLoad::stateIdle;
}

void SceneGhostLoad::nextScene() {
    m_state = &SceneGhostLoad::stateNextScene;
}

void SceneGhostLoad::stateWait() {
    if (m_selectSlot.isWaiting()) {
        return;
    }

    if (m_selectSlot.canLoad()) {
        slideIn();
    }
}

void SceneGhostLoad::stateSlideIn() {
    idle();
}

void SceneGhostLoad::stateSlideOut() {
    nextScene();
}

void SceneGhostLoad::stateIdle() {}

void SceneGhostLoad::stateNextScene() {
    if (!SequenceApp::Instance()->ready(m_nextScene)) {
        return;
    }

    SequenceApp::Instance()->setNextScene(m_nextScene);
}
