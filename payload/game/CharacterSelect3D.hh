#pragma once

#include <common/Array.hh>
#include <jsystem/JKRHeap.hh>
#include <payload/Replace.hh>

class CharacterSelect3D {
public:
    void REPLACED(draw)(s32 statusIndex, f32 f1);
    REPLACE void draw(s32 statusIndex, f32 f1);
    void REPLACED(calc)();
    REPLACE void calc();
    void REPLACED(setCharacter)(s32 statusIndex, s32 r5, s32 r6, void *r7, void *r8, f32 f1);
    REPLACE void setCharacter(s32 statusIndex, s32 r5, s32 r6, void *r7, void *r8, f32 f1);
    void REPLACED(setCharacterStatus)(s32 statusIndex, s32 r5, s32 r6);
    REPLACE void setCharacterStatus(s32 statusIndex, s32 r5, s32 r6);
    void REPLACED(setKart)(s32 statusIndex, s32 r5, void *r6, void *r7, f32 f1);
    REPLACE void setKart(s32 statusIndex, s32 r5, void *r6, void *r7, f32 f1);
    bool REPLACED(isCancel)(s32 statusIndex);
    REPLACE bool isCancel(s32 statusIndex);
    bool REPLACED(isNext)(s32 statusIndex);
    REPLACE bool isNext(s32 statusIndex);

    static CharacterSelect3D *Create(JKRHeap *heap);
    static void Destroy();
    static CharacterSelect3D *Instance();

private:
    CharacterSelect3D(JKRHeap *heap);
    ~CharacterSelect3D();

    u8 _0000[0x1018 - 0x0000];

    static CharacterSelect3D *s_instance;
    static JKRHeap *s_extraHeap;
    static Array<CharacterSelect3D *, 3> s_extraInstances;
};
size_assert(CharacterSelect3D, 0x1018);
