#include "SceneCardSelect.hh"

#include "game/AppMgr.hh"
#include "game/GhostAction.hh"
#include "game/KartGamePad.hh"
#include "game/MenuBackground.hh"
#include "game/MenuTitleLine.hh"
#include "game/RaceApp.hh"
#include "game/SequenceApp.hh"
#include "game/SequenceInfo.hh"

#include <jsystem/J2DAnmLoaderDataBase.hh>

SceneCardSelect::SceneCardSelect(JKRArchive *archive, JKRHeap *heap) : Scene(archive, heap) {
    m_screen.set("SelectMemoryCard.blo", 0x20000, m_archive);

    for (u32 i = 0; i < m_cardAnmTransforms.count(); i++) {
        m_cardAnmTransforms[i] = J2DAnmLoaderDataBase::Load("SelectMemoryCard.bck", m_archive);
        m_screen.search("MeSlot_%c", "AB"[i])->setAnimation(m_cardAnmTransforms[i]);
    }
    m_skipAnmTransform = J2DAnmLoaderDataBase::Load("SelectMemoryCard.bck", m_archive);
    m_screen.search("NMemQuit")->setAnimation(m_skipAnmTransform);

    m_cardAnmTransformFrames.fill(0);
    m_skipAnmTransformFrame = 0;
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

    for (u32 i = 0; i < m_cardAnmTransforms.count(); i++) {
        m_cardAnmTransforms[i]->m_frame = m_cardAnmTransformFrames[i];
    }
    m_skipAnmTransform->m_frame = m_skipAnmTransformFrame;

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
    if (m_skipAnmTransformFrame < 9) {
        m_skipAnmTransformFrame++;
        m_cardAnmTransformFrames.fill(m_skipAnmTransformFrame);
    } else {
        idle();
    }
}

void SceneCardSelect::stateSlideOut() {
    if (m_skipAnmTransformFrame > 0) {
        m_skipAnmTransformFrame--;
        m_cardAnmTransformFrames.fill(m_skipAnmTransformFrame);
    } else {
        if (m_nextScene == SceneType::None) {
            nextRace();
        } else {
            nextScene();
        }
    }
}

void SceneCardSelect::stateIdle() {
    const JUTGamePad::CButton &button = KartGamePad::GamePad(0)->button();
    if (button.risingEdge() & PAD_BUTTON_A) {
    } else if (button.risingEdge() & PAD_BUTTON_B) {
        m_nextScene = SceneType::CourseSelect;
        slideOut();
    }
}

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
