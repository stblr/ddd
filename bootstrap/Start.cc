#include "bootstrap/Bootstrap.hh"
#include "bootstrap/BootstrapBinary.hh"

#include <common/Arena.hh>
extern "C" {
#include <common/StackCanary.h>

#include <string.h>
}

extern "C" u32 stackTop;

extern "C" void RunBootstrap() {
    void *bssStart = BootstrapBinary::BssSectionStart();
    size_t bssSize = BootstrapBinary::BssSectionSize();
    memset(bssStart, 0, bssSize);

    void (**ctorsStart)() = reinterpret_cast<void (**)()>(BootstrapBinary::CtorsSectionStart());
    void (**ctorsEnd)() = reinterpret_cast<void (**)()>(BootstrapBinary::CtorsSectionEnd());
    for (void (**ctor)() = ctorsStart; ctor < ctorsEnd; ctor++) {
        (*ctor)();
    }

    MEM2Arena::Init(0x90000000, 0x93400000);
    Bootstrap::Run();

    while (true) {} // We should never get there
}

#ifdef __CWCC__
extern "C" asm void Start() {
    // clang-format off

    nofralloc

    // Initialize the stack pointer
    lis r1, stackTop@h
    ori r1, r1, stackTop@l

    // Initialize the stack canary
    bl StackCanary_Init

    // Jump to C++ code
    bl RunBootstrap

    // Dummy: the presence of such an instruction is the criterion for Dolphin to load an ELF in Wii
    // mode instead of GameCube.
    mtspr 1011, r3

    // clang-format on
}
#endif