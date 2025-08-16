#pragma once

#include "game/Scene.hh"
#include "game/SelectSlot.hh"

class SceneGhostCheckSave : public Scene {
public:
    SceneGhostCheckSave(JKRArchive *archive, JKRHeap *heap);
    ~SceneGhostCheckSave() override;
    void init() override;
    void draw() override;
    void calc() override;

    SelectSlot &selectSlot();

    static SceneGhostCheckSave *Instance();

private:
    void REPLACED(checkCard)();
    REPLACE void checkCard();

    u8 _000c[0x0014 - 0x000c];
    SelectSlot m_selectSlot;
    u8 _2128[0x253c - 0x2128];
    u32 m_ghostAction;

    static SceneGhostCheckSave *s_instance;
};
size_assert(SceneGhostCheckSave, 0x2540);
