#include "loader/Loader.hh"

#include <common/Types.hh>

extern "C" {
#include <string.h>
}

extern "C" u32 stackTop;

extern "C" void RunLoader() {
    void *bssStart = Loader::BssSectionStart();
    size_t bssSize = Loader::BssSectionSize();
    memset(bssStart, 0, bssSize);

    void (**ctorsStart)() = reinterpret_cast<void (**)()>(Loader::CtorsSectionStart());
    void (**ctorsEnd)() = reinterpret_cast<void (**)()>(Loader::CtorsSectionEnd());
    for (void (**ctor)() = ctorsStart; ctor < ctorsEnd; ctor++) {
        (*ctor)();
    }

    Loader::PayloadEntryFunc payloadEntry = Loader::Run();
    if (payloadEntry) {
        (*payloadEntry)();
    }
}

extern "C" asm void Start() {
    // clang-format off

    nofralloc

    // Initialize the stack pointer
    lis r1, stackTop@h
    ori r1, r1, stackTop@l

    // Jump to C++ code
    bl RunLoader

    // We should never get there
loop:
    b loop

    // Dummy: the presence of such an instruction is the criterion for Dolphin to load an ELF in Wii
    // mode instead of GameCube.
    mtspr 1011, r3

    // clang-format on
}

void operator delete(void *) {
    while (true) {}
}
