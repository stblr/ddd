#pragma once

#include <jsystem/J2DScreen.hh>
#include <jsystem/JKRArchive.hh>
#include <jsystem/JKRHeap.hh>
#include <portable/Array.hh>

class SelectSlot {
public:
    class Action {
    public:
        enum {
            None = 0,
            Next = 1,
            Prev = 2,
            Skip = 3,
        };

    private:
        Action();
    };

    SelectSlot();
    ~SelectSlot();

    bool isWaiting() const;
    bool canLoad() const;
    bool isIdle() const;
    void setup(JKRArchive *archive, JKRHeap *heap);
    void init();
    void draw(const J2DGraphContext *graphContext);
    void processCards();
    void calcAnm();
    void initLoad();
    void frameIn();
    void frameOut();
    u32 selectSlot();

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
        bool hasFrameInAnm() const;
        bool hasFrameOutAnm() const;
        void calcAnm();
        void select();
        void frameIn();
        void frameOut();

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

        class AnmState {
        public:
            enum {
                FrameIn = 1,
                FrameOut = 2,
            };

        private:
            AnmState();
        };

        u32 m_state;
        u8 _0004[0x000c - 0x0004];
        GhostFileInfoTable m_fileInfoTable;
        u32 m_anmState;
        u8 _0ff8[0x1064 - 0x0ff8];
    };
    size_assert(Card, 0x1064);

    class QuitBtn {
    public:
        QuitBtn();
        ~QuitBtn();

        bool hasFrameInAnm() const;
        bool hasFrameOutAnm() const;
        void calcAnm();
        void deselect();
        void frameIn();
        void frameOut();

    private:
        class State {
        public:
            enum {
                Selected = 0,
                Selectable = 1,
            };

        private:
            State();
        };

        class AnmState {
        public:
            enum {
                FrameIn = 1,
                FrameOut = 2,
            };

        private:
            AnmState();
        };

        u32 m_state;
        u32 m_anmState;
        u8 _08[0x3a - 0x08];
    };
    size_assert(QuitBtn, 0x3c);

    class State {
    public:
        enum {
            Wait = 0,
            FrameIn = 1,
            Idle = 2,
            FrameOut = 3,
        };

    private:
        State();
    };

    bool hasFrameInAnm() const;
    bool hasFrameOutAnm() const;

    J2DScreen *m_screen;
    Array<Card, 2> m_cards;
    QuitBtn m_quitBtn;
    u32 m_state;
    J2DAnmBase *m_anmTransform;
    s16 m_anmTransformFrame;
    u8 m_slotIndex;
    bool m_isQuitBtnSelected;
};
size_assert(SelectSlot, 0x2114);
