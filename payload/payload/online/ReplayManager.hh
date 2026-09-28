#pragma once

#include "payload/StorageScanner.hh"

#include <formats/Online.hh>
#include <portable/Ring.hh>
#include <portable/crypto/Types.hh>

const u32 MaxReplayCount = 56;

class ReplayManager
    : public StorageScanner
    , public ReplayReader<ReplayManager>
    , public ReplayRaceReader<ReplayManager>
    , public ReplayClientReader<ReplayManager>
    , public ClientPlayerReader<ReplayManager> {
public:
    struct Player {
        Array<char, PlayerNameLength + 1> name;
    };

    struct Client {
        Ring<Player, MaxClientPlayerCount> players;
        u8 kartCount;
    };

    struct Replay {
        u8 modeIndex;
        u8 packCourseCount;
        Hash packHash;
        u8 courseIndex;
        Ring<Client, MaxRoomClientCount> clients;
        s64 time;
        u8 kartCount;
    };

    u32 replayCount() const;
    const Replay &replay(u32 index) const;

    bool isMagicValid(u32 magic);
    void setMagic(u32 magic);
    bool isProtocolVersionValid(u16 protocolVersion);
    void setProtocolVersion(u16 protocolVersion);
    bool isReservedValid(u16 reserved);
    void setReserved(u16 reserved);

    bool isFrameRateValid(u8 frameRate);
    void setFrameRate(u8 frameRate);
    bool isModeIndexValid(u8 modeIndex);
    void setModeIndex(u8 modeIndex);
    bool isPackCourseCountValid(u8 packCourseCount);
    void setPackCourseCount(u8 packCourseCount);
    bool isPackHashElementValid(u32 i0, u8 packHashElement);
    void setPackHashElement(u32 i0, u8 packHashElement);
    bool isCourseIndexValid(u8 courseIndex);
    void setCourseIndex(u8 courseIndex);
    bool isClientsCountValid(u32 clientsCount);
    void setClientsCount(u32 clientsCount);
    ReplayClientReader *clientsElementReader(u32 i0);
    bool isTimeValid(u64 time);
    void setTime(u64 time);

    bool isPkElementValid(u32 i0, u8 pkElement);
    void setPkElement(u32 i0, u8 pkElement);
    bool isRegionValid(u8 region);
    void setRegion(u8 region);
    bool isPlatformCountValid(u32 platformCount);
    void setPlatformCount(u32 platformCount);
    bool isPlatformElementValid(u32 i0, u8 platformElement);
    void setPlatformElement(u32 i0, u8 platformElement);
    bool isPlayersCountValid(u32 playersCount);
    void setPlayersCount(u32 playersCount);
    ClientPlayerReader *playersElementReader(u32 i0);
    bool isKartCountValid(u8 kartCount);
    void setKartCount(u8 kartCount);

    bool isProfileValid(u8 profile);
    void setProfile(u8 profile);
    bool isNameElementValid(u32 i0, u8 nameElement);
    void setNameElement(u32 i0, u8 nameElement);

    static void Init();
    static ReplayManager *Instance();

private:
    ReplayManager();

    OSThread &thread() override;
    void process() override;

    void addReplays(Array<char, 256> &path, Storage::NodeInfo &nodeInfo);
    void addReplay(const Array<char, 256> &path);

    Ring<Replay, MaxReplayCount> m_replays;
    Replay *m_replay;
    u32 m_clientIndex;
    u32 m_playerIndex;
    Array<u8, 4 * 1024> m_stack;
    OSThread m_thread;

    static ReplayManager *s_instance;
};
