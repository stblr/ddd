#pragma once

#include "game/Scene.hh"

class SceneGhostLoad : public Scene {
public:
    SceneGhostLoad(JKRArchive *archive, JKRHeap *heap);
    ~SceneGhostLoad() override;
    void init() override;
    void draw() override;
    void calc() override;

private:
    typedef void (SceneGhostLoad::*State)();

    void slideIn();
    void slideOut();
    void idle();
    void nextScene();

    void stateSlideIn();
    void stateSlideOut();
    void stateIdle();
    void stateNextScene();

    State m_state;
    u32 m_nextScene;
};
