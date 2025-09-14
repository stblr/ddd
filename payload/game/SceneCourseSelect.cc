#include "SceneCourseSelect.hh"

#include "game/ResMgr.hh"
#include "game/SceneType.hh"
#include "game/SequenceApp.hh"
#include "game/SequenceInfo.hh"

void SceneCourseSelect::init() {
    m_underScreen->search("N_Remove")->setHasARScissor(true, true);
    m_underScreen->search("Cwhite01")->setHasARScissor(true, false);
    m_underScreen->search("Cwhite02")->setHasARScissor(true, false);

    if (SequenceApp::Instance()->prevScene() == SceneType::CardSelect) {
        SequenceInfo::Instance().m_fromPause = true;
    }

    REPLACED(init)();
}

void SceneCourseSelect::nextScene() {
    if (m_nextScene == SceneType::GhostCheckSave) {
        if (!ResMgr::IsFinishedLoadingArc(ResMgr::ArchiveID::Course)) {
            return;
        }

        if (!SequenceApp::Instance()->checkFinishAllLoading()) {
            return;
        }

        m_nextScene = SceneType::CardSelect;
    }

    REPLACED(nextScene)();
}
