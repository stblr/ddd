#pragma once

#include "game/Scene.hh"

#include <jsystem/J2DScreen.hh>
#include <portable/Array.hh>

class SceneNameSelect : public Scene {
public:
    SceneNameSelect(JKRArchive *archive, JKRHeap *heap);
    ~SceneNameSelect() override;
    void init() override;
    void draw() override;
    void calc() override;

private:
    enum {
        NameCount = 4,
    };

    typedef void (SceneNameSelect::*State)();

    void slideIn();
    void slideOut();
    void idle();
    void wait();
    void nextScene();

    void stateSlideIn();
    void stateSlideOut();
    void stateIdle();
    void stateWait();
    void stateNextScene();

    State m_state;
    u32 m_padCount;
    u32 m_nextScene;
    J2DScreen m_mainScreen;
    J2DScreen m_padCountScreen;
    Array<J2DScreen, NameCount> m_nameScreens;
    J2DAnmBase *m_mainAnmTransform;
    J2DAnmBase *m_nameAnmTransform;
    J2DAnmBase *m_padCountAnmTransform;
    J2DAnmBase *m_padCountCircleAnmTransform;
    u8 m_mainAnmTransformFrame;
    u8 m_nameAnmTransformFrame;
    u8 m_padCountAnmTransformFrame;
    u8 m_padCountCircleAnmTransformFrame;
};
