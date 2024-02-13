#include "CharacterSelect3D.hh"

#include <common/Arena.hh>
#include <jsystem/JKRExpHeap.hh>

void CharacterSelect3D::draw(s32 statusIndex, f32 f1) {
    if (statusIndex >= 1 && s_extraInstances[statusIndex - 1]) {
        s_extraInstances[statusIndex - 1]->REPLACED(draw)(0, f1);
    } else {
        REPLACED(draw)(statusIndex, f1);
    }
}

void CharacterSelect3D::calc() {
    REPLACED(calc)();

    for (u32 i = 0; i < s_extraInstances.count(); i++) {
        if (s_extraInstances[i]) {
            s_extraInstances[i]->REPLACED(calc)();
        }
    }
}

void CharacterSelect3D::setCharacter(s32 statusIndex, s32 r5, s32 r6, void *r7, void *r8, f32 f1) {
    if (statusIndex >= 1 && s_extraInstances[statusIndex - 1]) {
        s_extraInstances[statusIndex - 1]->REPLACED(setCharacter)(0, r5, r6, r7, r8, f1);
    } else {
        REPLACED(setCharacter)(statusIndex, r5, r6, r7, r8, f1);
    }
}

void CharacterSelect3D::setCharacterStatus(s32 statusIndex, s32 r5, s32 r6) {
    if (statusIndex >= 1 && s_extraInstances[statusIndex - 1]) {
        s_extraInstances[statusIndex - 1]->REPLACED(setCharacterStatus)(0, r5, r6);
    } else {
        REPLACED(setCharacterStatus)(statusIndex, r5, r6);
    }
}

void CharacterSelect3D::setKart(s32 statusIndex, s32 r5, void *r6, void *r7, f32 f1) {
    if (statusIndex >= 1 && s_extraInstances[statusIndex - 1]) {
        s_extraInstances[statusIndex - 1]->REPLACED(setKart)(0, r5, r6, r7, f1);
    } else {
        REPLACED(setKart)(statusIndex, r5, r6, r7, f1);
    }
}

bool CharacterSelect3D::isCancel(s32 statusIndex) {
    if (statusIndex >= 1 && s_extraInstances[statusIndex - 1]) {
        return s_extraInstances[statusIndex - 1]->REPLACED(isCancel)(0);
    } else {
        return REPLACED(isCancel)(statusIndex);
    }
}

bool CharacterSelect3D::isNext(s32 statusIndex) {
    if (statusIndex >= 1 && s_extraInstances[statusIndex - 1]) {
        return s_extraInstances[statusIndex - 1]->REPLACED(isNext)(0);
    } else {
        return REPLACED(isNext)(statusIndex);
    }
}

CharacterSelect3D *CharacterSelect3D::Create(JKRHeap *heap) {
    if (!s_instance) {
        s_instance = new (heap, 0x4) CharacterSelect3D(heap);
        if (!s_extraHeap) {
            size_t extraHeapSize = 0x200000;
            void *extraHeap = MEM2Arena::Instance()->alloc(extraHeapSize, 0x4);
            JKRHeap *parentHeap = JKRHeap::GetRootHeap();
            s_extraHeap = JKRExpHeap::Create(extraHeap, extraHeapSize, parentHeap, false);
        }
        for (u32 i = 0; i < s_extraInstances.count(); i++) {
            JKRHeap *heap = s_extraHeap->becomeCurrentHeap();
            s_extraInstances[i] = new (s_extraHeap, 0x4) CharacterSelect3D(s_extraHeap);
            heap->becomeCurrentHeap();
        }
    }
    return s_instance;
}

void CharacterSelect3D::Destroy() {
    for (u32 i = s_extraInstances.count(); i-- > 0;) {
        delete s_extraInstances[i];
        s_extraInstances[i] = nullptr;
    }
    if (s_extraHeap) {
        s_extraHeap->freeAll();
    }
    delete s_instance;
    s_instance = nullptr;
}

CharacterSelect3D *CharacterSelect3D::Instance() {
    return s_instance;
}

JKRHeap *CharacterSelect3D::s_extraHeap = nullptr;
Array<CharacterSelect3D *, 3> CharacterSelect3D::s_extraInstances(nullptr);
