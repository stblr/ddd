#include "SceneCardSelect.hh"

#include "game/AppMgr.hh"
#include "game/GhostAction.hh"
#include "game/Kart2DCommon.hh"
#include "game/KartGamePad.hh"
#include "game/MenuBackground.hh"
#include "game/MenuTitleLine.hh"
#include "game/RaceApp.hh"
#include "game/SequenceApp.hh"
#include "game/SequenceInfo.hh"

#include <jsystem/J2DAnmLoaderDataBase.hh>

SceneCardSelect::SceneCardSelect(JKRArchive *archive, JKRHeap *heap) : Scene(archive, heap) {
    m_cardScreen.set("SelectMemoryCard.blo", 0x20000, m_archive);
    m_ghostLayoutScreen.set("GDIndexLayout.blo", 0x20000, m_archive);
    for (u32 i = 0; i < m_ghostScreens.count(); i++) {
        m_ghostScreens[i].set("GDIndexLine.blo", 0x20000, m_archive);
    }
    m_buttonLayoutScreen.set("LoadGhost.blo", 0x20000, m_archive);
    for (u32 i = 0; i < m_buttonScreens.count(); i++) {
        m_buttonScreens[i].set("LoadGhostButton.blo", 0x20000, m_archive);
    }

    for (u32 i = 0; i < m_buttonScreens.count(); i++) {
        m_buttonLayoutScreen.search("Button%u", i)->appendChild(&m_buttonScreens[i]);
    }

    for (u32 i = 0; i < m_cardAnmTransforms.count(); i++) {
        m_cardAnmTransforms[i] = J2DAnmLoaderDataBase::Load("SelectMemoryCard.bck", m_archive);
        m_cardScreen.search("MeSlot_%c", "AB"[i])->setAnimation(m_cardAnmTransforms[i]);
    }
    m_skipAnmTransform = J2DAnmLoaderDataBase::Load("SelectMemoryCard.bck", m_archive);
    m_cardScreen.search("NMemQuit")->setAnimation(m_skipAnmTransform);

    m_cardAnmTransformFrames.fill(0);
    m_skipAnmTransformFrame = 0;
}

SceneCardSelect::~SceneCardSelect() {}

void SceneCardSelect::init() {
    Kart2DCommon *kart2DCommon = Kart2DCommon::Instance();
    J2DPane *pane = m_buttonScreens[0].search("TextO");
    kart2DCommon->changeUnicodeTexture("Race 5 ghosts", 23, m_buttonScreens[0], "Name", pane);
    pane = m_buttonScreens[1].search("TextO");
    kart2DCommon->changeUnicodeTexture("Watch 5 ghosts", 23, m_buttonScreens[1], "Name", pane);

    wait();
}

void SceneCardSelect::draw() {
    m_graphContext->setViewport();

    MenuBackground::Instance()->draw(m_graphContext);
    MenuTitleLine::Instance()->draw(m_graphContext);

    m_cardScreen.draw(0.0f, 0.0f, m_graphContext);
    m_ghostLayoutScreen.draw(0.0f, 0.0f, m_graphContext);
    m_buttonLayoutScreen.draw(0.0f, 0.0f, m_graphContext);
}

void SceneCardSelect::calc() {
    (this->*m_state)();

    MenuBackground::Instance()->calc();
    MenuTitleLine::Instance()->calc();

    for (u32 i = 0; i < m_cardAnmTransforms.count(); i++) {
        m_cardAnmTransforms[i]->m_frame = m_cardAnmTransformFrames[i];
    }
    m_skipAnmTransform->m_frame = m_skipAnmTransformFrame;

    m_cardScreen.animation();
    m_ghostLayoutScreen.animation();
    for (u32 i = 0; i < m_ghostScreens.count(); i++) {
        m_ghostScreens[i].animationMaterials();
    }
    m_buttonLayoutScreen.animation();
    for (u32 i = 0; i < m_buttonScreens.count(); i++) {
        m_buttonScreens[i].animationMaterials();
    }
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
