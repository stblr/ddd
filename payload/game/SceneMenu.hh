#pragma once

#include "game/Scene.hh"

#include <common/Array.hh>
#include <payload/Replace.hh>

class SceneMenu : public Scene {
public:
    SceneMenu(JKRArchive *archive, JKRHeap *heap);
    ~SceneMenu() override;
    void REPLACED(init)();
    REPLACE void init() override;
    void draw() override;
    void calc() override;

private:
    struct Config {
        s32 characterIndex; // This field is per pad
        Array<s32, 2> padIndices;
        Array<s32, 2> characterIndices;
        s32 kartIndex;
        s32 state;
    };
    size_assert(Config, 0x1c);

    void REPLACED(reset)();
    REPLACE void reset();
    REPLACE void selectUp(s32 padIndex);
    REPLACE void selectDown(s32 padIndex);
    void REPLACED(selectLeft)(s32 padIndex);
    REPLACE void selectLeft(s32 padIndex);
    void REPLACED(selectRight)(s32 padIndex);
    REPLACE void selectRight(s32 padIndex);
    bool selectCommon(s32 padIndex);
    bool isSelectAnm(s32 padIndex);
    void setCharacterFlags(u32 padIndex);
    void setCharacterFlag(s32 characterIndex, bool flag);
    void REPLACED(setRaceData)();
    REPLACE void setRaceData();

    u8 _000c[0x20f0 - 0x000c];
    u32 m_nextScene;
    u8 _20f4[0x211c - 0x20f4];
    Array<u32, 4> m_padStatuses;
    Array<Config, 4> m_configs;
    u8 _219c[0x228c - 0x219c];
    Array<bool, 0x14> m_characterFlags;
    u8 _22a0[0x22c0 - 0x22a0];
};
size_assert(SceneMenu, 0x22c0);
