#pragma once

#include <jsystem/J2DScreen.hh>
#include <jsystem/JKRArchive.hh>
#include <jsystem/JKRHeap.hh>
#include <portable/Array.hh>

class SelectSlot {
public:
    SelectSlot();
    ~SelectSlot();

    bool isWaiting() const;
    bool canLoad() const;
    void setup(JKRArchive *archive, JKRHeap *heap);
    void init();
    void draw(const J2DGraphContext *graphContext);
    void processCards();
    void calcAnm();
    void frameIn();

private:
    class GhostFileInfoTable {
    public:
        u8 count() const;

    private:
        u8 m_count;
        u8 _001[0xfe8 - 0x001];
    };
    size_assert(GhostFileInfoTable, 0xfe8);

    class Card {
    public:
        Card();
        ~Card();

        bool isWaiting() const;
        bool canLoad() const;
        void calcAnm();

    private:
        class State {
        public:
            enum {
                Mounting = 2,
                Checking = 3,
                Selectable = 4,
                Selected = 5,
            };

        private:
            State();
        };

        u32 m_state;
        u8 _0004[0x000c - 0x0004];
        GhostFileInfoTable m_fileInfoTable;
        u8 _0ff4[0x1064 - 0x0ff4];
    };
    size_assert(Card, 0x1064);

    J2DScreen *m_screen;
    Array<Card, 2> m_cards;
    u8 _20cc[0x2114 - 0x20cc];
};
size_assert(SelectSlot, 0x2114);
