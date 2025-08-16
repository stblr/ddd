#pragma once

#include "game/Scene.hh"

class SceneGhostCheckSave : public Scene {
public:
    SceneGhostCheckSave(JKRArchive *archive, JKRHeap *heap);
    ~SceneGhostCheckSave() override;
    void init() override;
    void draw() override;
    void calc() override;

    static SceneGhostCheckSave *Instance();

private:
    u8 _000c[0x253c - 0x000c];

public:
    u32 m_ghostAction;

private:
    static SceneGhostCheckSave *s_instance;
};
