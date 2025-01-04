#pragma once

#ifdef __CWCC__
#include "payload/Mutex.hh"
#endif

extern "C" {
#include <bearssl/bearssl.h>
}
#include <common/Types.hh>

class Random {
public:
    static void Init();
    static void Get(void *data, size_t size);
    static const br_prng_class **Ctx();

private:
    Random();

#ifdef __CWCC__
    static bool InitWithDiscTimings();
    static void InitWithES();
#endif

    static void Generate(const br_prng_class **ctx, void *out, size_t len);

#ifdef __CWCC__
    static bool s_isInit;
    static Mutex *s_mutex;
    static br_hmac_drbg_context s_ctx;
#endif
};
