#pragma once

#include "game/Scene.hh"

#include <jsystem/J2DScreen.hh>

class SceneCardSelect : public Scene {
public:
    SceneCardSelect(JKRArchive *archive, JKRHeap *heap);
    ~SceneCardSelect() override;
    void init() override;
    void draw() override;
    void calc() override;

private:
    typedef void (SceneCardSelect::*State)();

    void wait();
    void slideIn();
    void slideOut();
    void idle();
    void nextScene();
    void nextRace();

    void stateWait();
    void stateSlideIn();
    void stateSlideOut();
    void stateIdle();
    void stateNextScene();
    void stateNextRace();

    State m_state;
    u32 m_nextScene;
    J2DScreen m_screen;
};
