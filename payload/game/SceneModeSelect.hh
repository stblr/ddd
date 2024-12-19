#pragma once

#include "game/Scene.hh"

class SceneModeSelect : public Scene {
public:
    SceneModeSelect(JKRArchive *archive, JKRHeap *heap);
    ~SceneModeSelect() override;
    void init() override;
    void draw() override;
    void calc() override;
};
