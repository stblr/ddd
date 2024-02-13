#include "SceneMenu.hh"

#include "game/GameAudioMain.hh"
#include "game/KartGamePad.hh"
#include "game/RaceInfo.hh"
#include "game/RaceMode.hh"
#include "game/SceneType.hh"
#include "game/SystemRecord.hh"

#include <payload/CourseManager.hh>

void SceneMenu::init() {
    CourseManager::Instance()->unlock();

    REPLACED(init)();
}

void SceneMenu::reset() {
    SystemRecord::Instance().unlockAll();

    REPLACED(reset)();
}

void SceneMenu::selectUp(s32 padIndex) {
    if (!selectCommon(padIndex)) {
        return;
    }

    s32 characterIndex = m_configs[padIndex].characterIndex;
    const JUTGamePad::CButton &button = KartGamePad::GamePad(padIndex)->button();
    if (button.repeat() & JUTGamePad::PAD_MSTICK_LEFT) {
        if (characterIndex & 1) {
            characterIndex += m_characterFlags.count() - 2;
            characterIndex %= m_characterFlags.count();
            characterIndex ^= 1;
        }
    } else if (button.repeat() & JUTGamePad::PAD_MSTICK_RIGHT) {
        if (characterIndex & 1) {
            characterIndex += 2;
            characterIndex %= m_characterFlags.count();
            characterIndex ^= 1;
        }
    } else {
        characterIndex ^= 1;
    }
    if (m_characterFlags[characterIndex]) {
        if (characterIndex != m_configs[padIndex].characterIndex) {
            m_configs[padIndex].characterIndex = characterIndex;
            GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_CURSOL);
        }
    }
}

void SceneMenu::selectDown(s32 padIndex) {
    if (!selectCommon(padIndex)) {
        return;
    }

    s32 characterIndex = m_configs[padIndex].characterIndex;
    const JUTGamePad::CButton &button = KartGamePad::GamePad(padIndex)->button();
    if (button.repeat() & JUTGamePad::PAD_MSTICK_LEFT) {
        if (!(characterIndex & 1)) {
            characterIndex += m_characterFlags.count() - 2;
            characterIndex %= m_characterFlags.count();
            characterIndex ^= 1;
        }
    } else if (button.repeat() & JUTGamePad::PAD_MSTICK_RIGHT) {
        if (!(characterIndex & 1)) {
            characterIndex += 2;
            characterIndex %= m_characterFlags.count();
            characterIndex ^= 1;
        }
    } else {
        characterIndex ^= 1;
    }
    if (m_characterFlags[characterIndex]) {
        if (characterIndex != m_configs[padIndex].characterIndex) {
            m_configs[padIndex].characterIndex = characterIndex;
            GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_CURSOL);
        }
    }
}

void SceneMenu::selectLeft(s32 padIndex) {
    if (!selectCommon(padIndex)) {
        REPLACED(selectLeft)(padIndex);
        return;
    }

    s32 characterIndex = m_configs[padIndex].characterIndex;
    do {
        characterIndex += m_characterFlags.count() - 2;
        characterIndex %= m_characterFlags.count();
    } while (!m_characterFlags[characterIndex]);
    if (characterIndex != m_configs[padIndex].characterIndex) {
        m_configs[padIndex].characterIndex = characterIndex;
        GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_CURSOL);
    }
}

void SceneMenu::selectRight(s32 padIndex) {
    if (!selectCommon(padIndex)) {
        REPLACED(selectRight)(padIndex);
        return;
    }

    s32 characterIndex = m_configs[padIndex].characterIndex;
    do {
        characterIndex += 2;
        characterIndex %= m_characterFlags.count();
    } while (!m_characterFlags[characterIndex]);
    if (characterIndex != m_configs[padIndex].characterIndex) {
        m_configs[padIndex].characterIndex = characterIndex;
        GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_CURSOL);
    }
}

bool SceneMenu::selectCommon(s32 padIndex) {
    if (isSelectAnm(padIndex)) {
        return false;
    }

    if (m_configs[m_padStatuses[padIndex]].state != 0) {
        return false;
    }

    if (m_configs[padIndex].characterIndex == -1) {
        return false;
    }

    setCharacterFlags(padIndex);
    return true;
}

void SceneMenu::setCharacterFlags(u32 padIndex) {
    m_characterFlags.fill(true);

    u32 status = m_padStatuses[padIndex];
    for (u32 otherPadIndex = 0; otherPadIndex < m_padStatuses.count(); otherPadIndex++) {
        if (otherPadIndex == padIndex) {
            continue;
        }

        if (m_padStatuses[otherPadIndex] != status) {
            continue;
        }

        setCharacterFlag(m_configs[otherPadIndex].characterIndex, false);
    }
    for (u32 i = 0; i < m_configs[status].characterIndices.count(); i++) {
        setCharacterFlag(m_configs[status].characterIndices[i], false);
    }
}

void SceneMenu::setCharacterFlag(s32 characterIndex, bool flag) {
    if (characterIndex >= 0 && static_cast<u32>(characterIndex) < m_characterFlags.count()) {
        m_characterFlags[characterIndex] = flag;
    }
}

void SceneMenu::setRaceData() {
    REPLACED(setRaceData)();

    if (RaceInfo::Instance().getRaceMode() == RaceMode::VS) {
        m_nextScene = SceneType::PackSelect;
    }
}
