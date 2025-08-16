#include "SceneGhostCheckSave.hh"

#include "game/GhostAction.hh"
#include "game/SequenceApp.hh"

SelectSlot &SceneGhostCheckSave::selectSlot() {
    return m_selectSlot;
}

void SceneGhostCheckSave::checkCard() {
    if (m_ghostAction == GhostAction::Load) {
        u32 nextScene = SceneType::GhostLoad;
        if (SequenceApp::Instance()->ready(nextScene)) {
            return;
        }

        SequenceApp::Instance()->setNextScene(nextScene);
        return;
    }

    REPLACED(checkCard)();
}

SceneGhostCheckSave *SceneGhostCheckSave::Instance() {
    return s_instance;
}
