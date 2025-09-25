#include "SceneNameSelect.hh"

#include "game/CardAgent.hh"
#include "game/Kart2DCommon.hh"
#include "game/MenuTitleLine.hh"
#include "game/OnlineBackground.hh"
#include "game/SceneFactory.hh"
#include "game/SequenceApp.hh"
#include "game/SequenceInfo.hh"

#include <jsystem/J2DAnmLoaderDataBase.hh>
#include <payload/online/Client.hh>
#include <payload/online/CubeServerManager.hh>

extern "C" {
#include <stdio.h>
}

SceneNameSelect::SceneNameSelect(JKRArchive *archive, JKRHeap *heap) : Scene(archive, heap) {
    SceneFactory *sceneFactory = SceneFactory::Instance();
    JKRArchive *lanEntryArchive = sceneFactory->archive(SceneFactory::ArchiveType::LanEntry);

    m_mainScreen.set("SelectName.blo", 0x1040000, lanEntryArchive);
    m_padCountScreen.set("PlayerIcon.blo", 0x1040000, m_archive);
    for (u32 i = 0; i < m_nameScreens.count(); i++) {
        m_nameScreens[i].set("SelectNameName.blo", 0x1040000, m_archive);
    }

    m_padCountScreen.search("Ns1234")->m_isVisible = false;
    m_padCountScreen.search("Ns12_3_4")->m_isVisible = false;
    m_padCountScreen.search("Ns12_34")->m_isVisible = false;

    for (u32 i = 0; i < m_nameScreens.count(); i++) {
        m_mainScreen.search("ENplay%u", i + 1)->appendChild(&m_nameScreens[i]);
    }

    m_mainAnmTransform = J2DAnmLoaderDataBase::Load("SelectName.bck", lanEntryArchive);
    m_mainScreen.search("N_Entry")->setAnimation(m_mainAnmTransform);
    m_nameAnmTransform = J2DAnmLoaderDataBase::Load("SelectName.bck", lanEntryArchive);
    for (u32 i = 0; i < 4; i++) {
        m_mainScreen.search("ENplay%u", i + 1)->setAnimation(m_nameAnmTransform);
    }
    for (u32 i = 0; i < m_outlineAnmTevRegKeys.count(); i++) {
        m_outlineAnmTevRegKeys[i] =
                J2DAnmLoaderDataBase::Load("select_character_cc.brk", m_archive);
        m_outlineAnmTevRegKeys[i]->searchUpdateMaterialID(&m_nameScreens[2]);
        m_nameScreens[i].search("CWC1")->setAnimation(m_outlineAnmTevRegKeys[i]);
    }
    for (u32 i = 0; i < m_padAnmColors.count(); i++) {
        m_padAnmColors[i] = J2DAnmLoaderDataBase::Load("select_character_cc.bpk", m_archive);
        m_padAnmColors[i]->searchUpdateMaterialID(&m_nameScreens[i]);
        m_nameScreens[i].search("CC1")->setAnimation(m_padAnmColors[i]);
    }
    m_padCountAnmTransform = J2DAnmLoaderDataBase::Load("PlayerIcon.bck", m_archive);
    m_padCountScreen.search("N_Stok")->setAnimation(m_padCountAnmTransform);
    m_padCountCircleAnmTransform = J2DAnmLoaderDataBase::Load("PlayerIcon.bck", m_archive);
    m_padCountScreen.search("Cstok_pb")->setAnimation(m_padCountCircleAnmTransform);

    m_padCountCircleAnmTransformFrame = 0;
}

SceneNameSelect::~SceneNameSelect() {}

void SceneNameSelect::init() {
    if (SequenceApp::Instance()->prevScene() == SceneType::HowManyPlayers) {
        m_padCount = SequenceInfo::Instance().m_padCount;

        Kart2DCommon *kart2DCommon = Kart2DCommon::Instance();
        for (u32 i = 0; i < m_nameScreens.count(); i++) {
            kart2DCommon->changeUnicodeTexture("ABC", 3, m_nameScreens[i], "CP");
            if (i < 4) {
                J2DPicture *picture = m_nameScreens[i].search("CC1")->downcast<J2DPicture>();
                Array<char, 32> name;
                snprintf(name.values(), name.count(), "SC_P%lu_u.bti", i + 1);
                picture->changeTexture(name.values(), 0);
                m_outlineAnmTevRegKeyFrames[i] = i + 1;
                m_padAnmColorFrames[i] = i + 1;
            } else {
                m_outlineAnmTevRegKeyFrames[i] = 0;
                m_padAnmColorFrames[i] = 0;
            }
        }

        J2DPicture *picture = m_padCountScreen.search("Cstok_p")->downcast<J2DPicture>();
        Array<char, 32> name;
        snprintf(name.values(), name.count(), "Player%lu.bti", m_padCount);
        picture->changeTexture(name.values(), 0);

        SequenceApp::Instance()->ready(SceneType::ServerSelect);
    }

    slideIn();
}

void SceneNameSelect::draw() {
    m_graphContext->setViewport();

    OnlineBackground::Instance()->draw(m_graphContext);
    MenuTitleLine::Instance()->draw(m_graphContext);

    m_mainScreen.draw(0.0f, 0.0f, m_graphContext);
    //m_padCountScreen.draw(0.0f, 0.0f, m_graphContext);
}

void SceneNameSelect::calc() {
    (this->*m_state)();

    OnlineBackground::Instance()->calc();
    MenuTitleLine::Instance()->calc();

    m_nameAnmTransformFrame = m_padCount;
    m_padCountCircleAnmTransformFrame = 14 + (m_padCountCircleAnmTransformFrame - 13) % 60;

    m_mainAnmTransform->m_frame = m_mainAnmTransformFrame;
    m_nameAnmTransform->m_frame = m_nameAnmTransformFrame;
    for (u32 i = 0; i < m_outlineAnmTevRegKeys.count(); i++) {
        m_outlineAnmTevRegKeys[i]->m_frame = m_outlineAnmTevRegKeyFrames[i];
    }
    for (u32 i = 0; i < m_padAnmColors.count(); i++) {
        m_padAnmColors[i]->m_frame = m_padAnmColorFrames[i];
    }
    m_padCountAnmTransform->m_frame = m_padCountAnmTransformFrame;
    m_padCountCircleAnmTransform->m_frame = m_padCountCircleAnmTransformFrame;

    m_mainScreen.animation();
    for (u32 i = 0; i < m_nameScreens.count(); i++) {
        m_nameScreens[i].animationMaterials();
    }
    m_padCountScreen.animation();
}

void SceneNameSelect::slideIn() {
    CubeServerManager::Instance()->unlock();
    Client::Instance()->reset();
    MenuTitleLine::Instance()->drop("SelectName.bti");
    m_mainAnmTransformFrame = 0;
    m_padCountAnmTransformFrame = 0;
    m_state = &SceneNameSelect::stateSlideIn;
}

void SceneNameSelect::slideOut() {
    MenuTitleLine::Instance()->lift();
    m_state = &SceneNameSelect::stateSlideOut;
}

void SceneNameSelect::idle() {
    m_state = &SceneNameSelect::stateIdle;
}

void SceneNameSelect::wait() {
    CardAgent::Ask(CardAgent::Command::WriteSystemFile, 0);
    m_state = &SceneNameSelect::stateWait;
}

void SceneNameSelect::nextScene() {
    m_state = &SceneNameSelect::stateNextScene;
}

void SceneNameSelect::stateSlideIn() {
    if (m_mainAnmTransformFrame < 15) {
        m_mainAnmTransformFrame++;
        if (m_mainAnmTransformFrame <= 9) {
            m_padCountAnmTransformFrame = m_mainAnmTransformFrame;
        }
    } else {
        idle();
    }
}

void SceneNameSelect::stateSlideOut() {
    if (m_mainAnmTransformFrame > 0) {
        m_mainAnmTransformFrame--;
        if (m_mainAnmTransformFrame <= 9) {
            m_padCountAnmTransformFrame = m_mainAnmTransformFrame;
        }
    } else {
        nextScene();
    }
}

void SceneNameSelect::stateIdle() {}

void SceneNameSelect::stateWait() {
    if (!CardAgent::IsReady()) {
        return;
    }

    CardAgent::Ack();
    slideOut();
}

void SceneNameSelect::stateNextScene() {
    if (!SequenceApp::Instance()->ready(m_nextScene)) {
        return;
    }

    SequenceApp::Instance()->setNextScene(m_nextScene);
}
