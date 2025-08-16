#pragma once

#include "game/Scene.hh"
#include "game/SelectSlot.hh"

class SceneGhostLoad : public Scene {
public:
    SceneGhostLoad(JKRArchive *archive, JKRHeap *heap);
    ~SceneGhostLoad() override;
    void init() override;
    void draw() override;
    void calc() override;

private:
    typedef void (SceneGhostLoad::*State)();

    void wait();
    void slideIn();
    void slideOut();
    void idle();
    void nextScene();

    void stateWait();
    void stateSlideIn();
    void stateSlideOut();
    void stateIdle();
    void stateNextScene();

    State m_state;
    u32 m_nextScene;
    SelectSlot m_selectSlot;
};
