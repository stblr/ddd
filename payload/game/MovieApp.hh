#pragma once

#include "game/GameApp.hh"

#include <payload/Replace.hh>

class MovieApp : public GameApp {
public:
    static MovieApp *REPLACED(Create)();
    REPLACE static MovieApp *Create();

private:
    MovieApp();
    void REPLACED(calc)();
    REPLACE void calc() override;

    u8 _00[0x10 - 0x0c];
    s32 m_state;
    u8 _14[0x2c - 0x14];
};
size_assert(MovieApp, 0x2c);
