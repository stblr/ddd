#include "SceneGhostLoad.hh"

#include "game/MenuBackground.hh"
#include "game/MenuTitleLine.hh"
#include "game/SequenceApp.hh"

SceneGhostLoad::SceneGhostLoad(JKRArchive *archive, JKRHeap *heap) : Scene(archive, heap) {}

SceneGhostLoad::~SceneGhostLoad() {}

void SceneGhostLoad::init() {
    slideIn();
}

void SceneGhostLoad::draw() {
    m_graphContext->setViewport();

    MenuBackground::Instance()->draw(m_graphContext);
    MenuTitleLine::Instance()->draw(m_graphContext);
}

void SceneGhostLoad::calc() {
    (this->*m_state)();

    MenuBackground::Instance()->calc();
    MenuTitleLine::Instance()->calc();
}

void SceneGhostLoad::slideIn() {
    MenuTitleLine::Instance()->drop(MenuTitleLine::Title::LoadGhostData);
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
