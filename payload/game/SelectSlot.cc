#include "SelectSlot.hh"

SelectSlot::SelectSlot() {}

SelectSlot::~SelectSlot() {}

bool SelectSlot::isWaiting() const {
    return m_cards.any(&Card::isWaiting);
}

bool SelectSlot::canLoad() const {
    return m_cards.any(&Card::canLoad);
}

void SelectSlot::draw(const J2DGraphContext *graphContext) {
    m_screen->draw(0.0f, 0.0f, graphContext);
}

void SelectSlot::calcAnm() {
    for (u32 i = 0; i < m_cards.count(); i++) {
        m_cards[i].calcAnm();
    }
    m_screen->animation();
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
