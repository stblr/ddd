#pragma once

extern "C" {
#include <bearssl/bearssl.h>
}
#include <common/Array.hh>

namespace DH {

class K {
public:
    K();
    ~K();

    br_ec_private_key m_k;
    Array<u8, 32> m_x;
};

class PK {
public:
    PK();
    explicit PK(const K &k);
    ~PK();

    br_ec_public_key m_pk;
    Array<u8, 32> m_q;
};

class SK {
public:
    SK(const K &k, const PK &pk);
    ~SK();

    Array<u8, 32> m_g;
};

} // namespace DH
