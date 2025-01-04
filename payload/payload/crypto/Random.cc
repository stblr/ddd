#include "Random.hh"

#ifdef __CWCC__
#include "payload/Lock.hh"

#include <common/Arena.hh>
#include <common/Array.hh>
#include <common/Clock.hh>
#include <common/ES.hh>
#include <common/Platform.hh>
#include <common/storage/Storage.hh>

extern "C" {
#include <assert.h>
#include <string.h>
}

void Random::Init() {
    assert(!s_isInit);

    s_mutex = new (MEM1Arena::Instance(), 0x4) Mutex;
    assert(s_mutex);

    if (Platform::IsGameCube()) {
        while (!InitWithDiscTimings()) {
            Clock::WaitSeconds(1);
        }
    } else {
        InitWithES();
    }

    s_isInit = true;
}

void Random::Get(void *data, size_t size) {
    assert(s_isInit);

    Lock<Mutex> lock(*s_mutex);
    br_hmac_drbg_generate(&s_ctx, data, size);
}
#else
#include <algorithm>
#include <climits>
#include <random>

void Random::Get(void *data, size_t size) {
    static std::independent_bits_engine<std::random_device, CHAR_BIT, u8> engine;
    std::generate(reinterpret_cast<u8 *>(data), reinterpret_cast<u8 *>(data) + size,
            std::ref(engine));
}
#endif

const br_prng_class **Random::Ctx() {
    static const br_prng_class vtable = {sizeof(const br_prng_class *), nullptr, Generate, nullptr};
    static const br_prng_class *context = &vtable;
    return &context;
}

#ifdef __CWCC__
bool Random::InitWithDiscTimings() {
    Storage::FileHandle file("dvd:/Movie/play1.thp", Storage::Mode::Read);
    alignas(0x20) Array<u8, 256> buffer;
    if (!file.read(buffer.values(), buffer.count(), 0)) {
        return false;
    }
    Array<u8, 32> seed;
    s64 start = Clock::GetMonotonicTicks();
    for (u32 i = 0; i < seed.count(); i++) {
        for (u32 j = 0; j < 8; j++) {
            if (!file.read(buffer.values(), buffer.count(), (1 + i) * 4096)) {
                return false;
            }
            s64 now = Clock::GetMonotonicTicks();
            seed[i] &= ~(1 << j);
            seed[i] |= ((now - start) & 1) << j;
            start = now;
        }
    }
    br_hmac_drbg_init(&s_ctx, &br_sha256_vtable, seed.values(), seed.count());
    return true;
}

void Random::InitWithES() {
    ES es;
    assert(es.ok());

    Array<u8, 0x3c> signature;
    Array<u8, 0x180> certificate;
    assert(es.sign(nullptr, 0, signature, certificate));
    br_hmac_drbg_init(&s_ctx, &br_sha256_vtable, signature.values(), 32);
}
#endif

void Random::Generate(const br_prng_class ** /* ctx */, void *out, size_t len) {
    Get(out, len);
}

#ifdef __CWCC__
bool Random::s_isInit = false;
Mutex *Random::s_mutex = nullptr;
br_hmac_drbg_context Random::s_ctx;
#endif
