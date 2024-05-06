#include "Random.hh"

#include <common/Algorithm.hh>
#include <common/Clock.hh>
#include <common/Platform.hh>
#include <common/storage/Storage.hh>
extern "C" {
#include <dolphin/OSTime.h>
#include <monocypher/monocypher.h>

#include <assert.h>
#include <string.h>
}

void Random::Init() {
    assert(!s_isInit);

    while (!InitWithDiscTimings()) {
        Clock::WaitMilliseconds(100);
    }
    s_isSecure = !Platform::IsDolphin();

    s_isInit = true;
}

bool Random::IsSecure() {
    return s_isSecure;
}

void Random::Get(void *data, size_t size) {
    assert(s_isInit);

    while (size > 0) {
        if (s_offset == s_buffer.count()) {
            Array<u8, 8> nonce(0);
            crypto_chacha20_djb(s_buffer.values(), nullptr, s_buffer.count(), s_buffer.values(),
                    nonce.values(), 0);
            s_offset = 32;
        }

        size_t chunkSize = Min(size, s_buffer.count() - s_offset);
        memcpy(data, s_buffer.values() + s_offset, chunkSize);
        data = reinterpret_cast<u8 *>(data) + chunkSize;
        size -= chunkSize;
        s_offset += chunkSize;
    }

    crypto_wipe(s_buffer.values() + 32, s_offset - 32);
}

bool Random::InitWithDiscTimings() {
    Storage::FileHandle file("dvd:/Movie/play1.thp", Storage::Mode::Read);
    alignas(0x20) Array<u8, 256> buffer;
    if (!file.read(buffer.values(), buffer.count(), 0)) {
        return false;
    }
    s64 start = OSGetTime();
    for (u32 i = 0; i < 32; i++) {
        for (u32 j = 0; j < 8; j++) {
            if (!file.read(buffer.values(), buffer.count(), (1 + i) * 4096)) {
                return false;
            }
            s64 now = OSGetTime();
            s_buffer[i] &= ~(1 << j);
            s_buffer[i] |= ((now - start) & 1) << j;
            start = now;
        }
    }
    return true;
}

bool Random::s_isInit = false;
bool Random::s_isSecure = false;
alignas(0x20) Array<u8, 32 + 256> Random::s_buffer;
u16 Random::s_offset = s_buffer.count();
