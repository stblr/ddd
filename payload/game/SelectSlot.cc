#include "SelectSlot.hh"

SelectSlot::SelectSlot() {}

SelectSlot::~SelectSlot() {}

bool SelectSlot::isWaiting() const {
    return m_cards.any(&Card::isWaiting);
}

bool SelectSlot::canLoad() const {
    return m_cards.any(&Card::canLoad);
}

bool SelectSlot::isIdle() const {
    return m_state == State::Idle;
}

void SelectSlot::draw(const J2DGraphContext *graphContext) {
    m_screen->draw(0.0f, 0.0f, graphContext);
}

void SelectSlot::calcAnm() {
    m_anmTransformFrame = (m_anmTransformFrame + 1) % 79;
    m_anmTransform->m_frame = m_anmTransformFrame;
    for (u32 i = 0; i < m_cards.count(); i++) {
        m_cards[i].calcAnm();
    }
    m_quitBtn.calcAnm();
    switch (m_state) {
    case State::FrameIn:
        if (!hasFrameInAnm()) {
            m_state = State::Idle;
        }
        break;
    case State::FrameOut:
        if (!hasFrameOutAnm()) {
            m_state = State::Wait;
        }
        break;
    }
    m_screen->animation();
}

void SelectSlot::initLoad() {
    for (u32 i = 0; i < m_cards.count(); i++) {
        if (m_cards[i].canLoad()) {
            m_cards[i].select();
            m_quitBtn.deselect();
            m_slotIndex = i;
            m_isQuitBtnSelected = false;
            return;
        }
    }
}

void SelectSlot::frameIn() {
    // TODO watchCardState?
    for (u32 i = 0; i < m_cards.count(); i++) {
        m_cards[i].frameIn();
    }
    m_quitBtn.frameIn();
    m_state = State::FrameIn;
}

void SelectSlot::frameOut() {
    for (u32 i = 0; i < m_cards.count(); i++) {
        m_cards[i].frameOut();
    }
    m_quitBtn.frameOut();
    m_state = State::FrameOut;
}

u8 SelectSlot::GhostFileInfoTable::count() const {
    return m_count;
}

SelectSlot::Card::~Card() {}

bool SelectSlot::Card::isWaiting() const {
    switch (m_state) {
    case State::Mounting:
    case State::Checking:
        return true;
    default:
        return false;
    }
}

bool SelectSlot::Card::canLoad() const {
    switch (m_state) {
    case State::Selectable:
    case State::Selected:
        return m_fileInfoTable.count() != 0;
    default:
        return false;
    }
}

bool SelectSlot::Card::hasFrameInAnm() const {
    return m_anmState == AnmState::FrameIn;
}

bool SelectSlot::Card::hasFrameOutAnm() const {
    return m_anmState == AnmState::FrameOut;
}

void SelectSlot::Card::select() {
    m_state = State::Selected;
}

void SelectSlot::Card::frameIn() {
    m_anmState = AnmState::FrameIn;
    // TODO set anm frame?
}

void SelectSlot::Card::frameOut() {
    m_anmState = AnmState::FrameOut;
    // TODO set anm frame?
}

SelectSlot::QuitBtn::QuitBtn() {}

SelectSlot::QuitBtn::~QuitBtn() {}

bool SelectSlot::QuitBtn::hasFrameInAnm() const {
    return m_anmState == AnmState::FrameIn;
}

bool SelectSlot::QuitBtn::hasFrameOutAnm() const {
    return m_anmState == AnmState::FrameOut;
}

void SelectSlot::QuitBtn::deselect() {
    m_state = State::Selectable;
}

void SelectSlot::QuitBtn::frameIn() {
    m_anmState = AnmState::FrameIn;
    // TODO set anm frame?
}

void SelectSlot::QuitBtn::frameOut() {
    m_anmState = AnmState::FrameOut;
    // TODO set anm frame?
}

bool SelectSlot::hasFrameInAnm() const {
    return m_cards.any(&Card::hasFrameInAnm) && m_quitBtn.hasFrameInAnm();
}

bool SelectSlot::hasFrameOutAnm() const {
    return m_cards.any(&Card::hasFrameOutAnm) && m_quitBtn.hasFrameOutAnm();
}
