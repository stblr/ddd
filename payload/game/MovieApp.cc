#include "MovieApp.hh"

#include "game/AppMgr.hh"

#include <portable/Log.hh>

MovieApp *MovieApp::Create() {
    switch (AppMgr::CurrentApp()) {
    case AppMgr::KartAppEnum::Title:
        // Allow the opening movie to be played from the 'Title' scene
        return REPLACED(Create)();
    default:
        return nullptr;
    }
}

void MovieApp::calc() {
    s32 state = m_state;

    REPLACED(calc)();

    if (state != 2 || m_state != 2) {
        DEBUG("%d %d", state, m_state);
    }
}
