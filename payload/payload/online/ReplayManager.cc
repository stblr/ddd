#include "ReplayManager.hh"

#include "payload/CourseManager.hh"

#include <cube/Arena.hh>
#include <cube/Clock.hh>
#include <game/Modes.hh>
#include <portable/Algorithm.hh>
#include <portable/Log.hh>

extern "C" {
#include <stdio.h>
#include <string.h>
}

void ReplayManager::filterAndSort() {
    CourseManager *courseManager = CourseManager::Instance();
    for (u32 i = 0; i < m_replays.count();) {
        Replay &replay = m_replays[i];
        u32 raceMode = Modes[replay.modeIndex];
        replay.isRace = RaceMode::IsRace(raceMode);
        Optional<u32> packIndex = courseManager->searchPack(true, replay.isRace,
                replay.packCourseCount, replay.packHash);
        if (packIndex &&
                replay.courseIndex < courseManager->courseCount(true, replay.isRace, *packIndex)) {
            replay.packIndex = *packIndex;
            i++;
        } else {
            m_replays.swapRemoveBack(i);
        }
    }
    Sort(m_replays, m_replays.count(), CompareReplaysByTime);
}

u32 ReplayManager::replayCount() const {
    return m_replays.count();
}

const ReplayManager::Replay &ReplayManager::replay(u32 index) const {
    return m_replays[index];
}

bool ReplayManager::isMagicValid(u32 magic) {
    return magic == ReplayMagic;
}

void ReplayManager::setMagic(u32 /* magic */) {}

bool ReplayManager::isProtocolVersionValid(u16 protocolVersion) {
    return protocolVersion == ProtocolVersion;
}

void ReplayManager::setProtocolVersion(u16 /* protocolVersion */) {}

bool ReplayManager::isReservedValid(u16 /* reserved */) {
    return true;
}

void ReplayManager::setReserved(u16 /*reserved */) {}

bool ReplayManager::isFrameRateValid(u8 /* frameRate */) {
    return true;
}

void ReplayManager::setFrameRate(u8 /* frameRate */) {}

bool ReplayManager::isModeIndexValid(u8 /* modeIndex */) {
    return true;
}

void ReplayManager::setModeIndex(u8 modeIndex) {
    m_replay->modeIndex = modeIndex;
}

bool ReplayManager::isPackCourseCountValid(u8 /* packCourseCount */) {
    return true;
}

void ReplayManager::setPackCourseCount(u8 packCourseCount) {
    m_replay->packCourseCount = packCourseCount;
}

bool ReplayManager::isPackHashElementValid(u32 /* i0 */, u8 /* packHashElement */) {
    return true;
}

void ReplayManager::setPackHashElement(u32 i0, u8 packHashElement) {
    m_replay->packHash[i0] = packHashElement;
}

bool ReplayManager::isCourseIndexValid(u8 /* courseIndex */) {
    return true;
}

void ReplayManager::setCourseIndex(u8 courseIndex) {
    m_replay->courseIndex = courseIndex;
}

bool ReplayManager::isClientsCountValid(u32 /* clientsCount */) {
    return true;
}

void ReplayManager::setClientsCount(u32 clientsCount) {
    m_replay->clients.reset();
    for (u32 i = 0; i < clientsCount; i++) {
        m_replay->clients.emplaceBack();
    }
}

ReplayClientReader<ReplayManager> *ReplayManager::clientsElementReader(u32 i0) {
    m_clientIndex = i0;
    return this;
}

bool ReplayManager::isTimeValid(u64 /* time */) {
    return true;
}

void ReplayManager::setTime(u64 time) {
    s64 epoch = 946684800; // 2000-01-01
    m_replay->time = Clock::SecondsToTicks(time - epoch);
}

bool ReplayManager::isPkElementValid(u32 /* i0 */, u8 /* pkElement */) {
    return true;
}

void ReplayManager::setPkElement(u32 /* i0 */, u8 /* pkElement */) {}

bool ReplayManager::isRegionValid(u8 /* region */) {
    return true;
}

void ReplayManager::setRegion(u8 /* region */) {}

bool ReplayManager::isPlatformCountValid(u32 /* platformCount */) {
    return true;
}

void ReplayManager::setPlatformCount(u32 /* platformCount */) {}

bool ReplayManager::isPlatformElementValid(u32 /* i0 */, u8 /* platformElement */) {
    return true;
}

void ReplayManager::setPlatformElement(u32 /* i0 */, u8 /* platformElement */) {}

bool ReplayManager::isPlayersCountValid(u32 /* playersCount */) {
    return true;
}

void ReplayManager::setPlayersCount(u32 playersCount) {
    m_replay->clients[m_clientIndex].players.reset();
    for (u32 i = 0; i < playersCount; i++) {
        Player *player = m_replay->clients[m_clientIndex].players.emplaceBack();
        player->name[PlayerNameLength] = '\0';
    }
}

ClientPlayerReader<ReplayManager> *ReplayManager::playersElementReader(u32 i0) {
    m_playerIndex = i0;
    return this;
}

bool ReplayManager::isTeamsCountValid(u32 /* teamsCount */) {
    return true;
}

void ReplayManager::setTeamsCount(u32 teamsCount) {
    m_replay->clients[m_clientIndex].teams.reset();
    for (u32 i = 0; i < teamsCount; i++) {
        m_replay->clients[m_clientIndex].teams.emplaceBack();
    }
}

bool ReplayManager::isTeamsElementValid(u32 /* i0 */, u8 /* teamsElement */) {
    return true;
}

void ReplayManager::setTeamsElement(u32 i0, u8 teamsElement) {
    m_replay->clients[m_clientIndex].teams[i0] = teamsElement;
}

bool ReplayManager::isProfileValid(u8 /* profile */) {
    return true;
}

void ReplayManager::setProfile(u8 /* profile */) {}

bool ReplayManager::isNameElementValid(u32 /* i0 */, u8 nameElement) {
    return nameElement != '\0';
}

void ReplayManager::setNameElement(u32 i0, u8 nameElement) {
    m_replay->clients[m_clientIndex].players[m_playerIndex].name[i0] = nameElement;
}

void ReplayManager::Init() {
    s_instance = new (MEM1Arena::Instance(), 0x4) ReplayManager;
}

ReplayManager *ReplayManager::Instance() {
    return s_instance;
}

ReplayManager::ReplayManager() {
    StorageScanner *param = this;
    OSCreateThread(&m_thread, Run, param, m_stack.values() + m_stack.count(), m_stack.count(), 27,
            0);
}

OSThread &ReplayManager::thread() {
    return m_thread;
}

void ReplayManager::process() {
    m_replays.reset();
    m_replay = m_replays.emplaceBack();
    Array<char, 256> path;
    snprintf(path.values(), path.count(), "main:/ddd/replays");
    Storage::CreateDir(path.values(), Storage::Mode::WriteAlways);
    Storage::NodeInfo nodeInfo;
    addReplays(path, nodeInfo);
    if (m_replay) {
        m_replays.popBack();
    }
}

void ReplayManager::addReplays(Array<char, 256> &path, Storage::NodeInfo &nodeInfo) {
    u32 length = strlen(path.values());
    for (Storage::DirHandle dir(path.values()); dir.read(nodeInfo);) {
        snprintf(path.values() + length, path.count() - length, "/%s", nodeInfo.name.values());
        if (nodeInfo.type == Storage::NodeType::Dir) {
            addReplays(path, nodeInfo);
        } else {
            addReplay(path);
        }
    }
    path[length] = '\0';
}

void ReplayManager::addReplay(const Array<char, 256> &path) {
    if (!m_replay) {
        return;
    }

    alignas(0x20) u8 buffer[512];
    u32 size;
    if (!Storage::ReadFile(path.values(), buffer, Count(buffer), &size)) {
        return;
    }

    u32 offset = 0;
    if (!ReplayReader::isValid(buffer, size, offset)) {
        return;
    }
    if (!ReplayRaceReader::isValid(buffer, size, offset)) {
        return;
    }
    offset = 0;
    ReplayReader::read(buffer, offset);
    ReplayRaceReader::read(buffer, offset);

    m_replay->kartCount = 0;
    for (u32 i = 0; i < m_replay->clients.count(); i++) {
        const Client &client = m_replay->clients[i];
        if (client.teams.count() * 2 < client.players.count() ||
                client.teams.count() > client.players.count()) {
            return;
        }
        m_replay->kartCount += client.teams.count();
    }
    if (m_replay->kartCount > MaxRoomKartCount) {
        return;
    }

    DEBUG("Adding replay %s...", path.values());
    m_replay = m_replays.emplaceBack();
}

bool ReplayManager::CompareReplaysByTime(const Replay &a, const Replay &b) {
    return a.time > b.time;
}

ReplayManager *ReplayManager::s_instance = nullptr;
