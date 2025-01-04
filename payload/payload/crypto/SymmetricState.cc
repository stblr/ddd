// clang-format off
//
// Based on https://github.com/blckngm/noise-rust/blob/master/noise-protocol/src/symmetricstate.rs
//
// clang-format on

#include "SymmetricState.hh"

#include "payload/crypto/HKDFState.hh"

extern "C" {
#include <bearssl/bearssl.h>
}
#include <common/Bytes.hh>
extern "C" {
#include <monocypher/monocypher.h>

#include <stdio.h>
#include <string.h>
}

SymmetricState::SymmetricState() {
    crypto_wipe(m_k.values(), m_k.count());
    m_nonce = UINT64_MAX;
    crypto_wipe(m_h.values(), m_h.count());
    snprintf(reinterpret_cast<char *>(m_h.values()), m_h.count(),
            "Noise_IK_25519_ChaChaPoly_BLAKE2b");
    m_ck = m_h;
}

SymmetricState::~SymmetricState() {
    crypto_wipe(m_ck.values(), m_ck.count());
    crypto_wipe(m_h.values(), m_h.count());
    crypto_wipe(m_k.values(), m_k.count());
}

void SymmetricState::mixDH(const DH::K &k, const DH::PK &pk) {
    DH::SK sk(k, pk);
    HKDFState hkdfState(m_ck, sk.m_g.values(), sk.m_g.count());
    hkdfState.update(m_ck.values(), m_ck.count());
    hkdfState.update(m_k.values(), m_k.count());
    m_nonce = 0;
}

void SymmetricState::mixHash(const u8 *input, size_t inputSize) {
    crypto_blake2b_ctx ctx;
    crypto_blake2b_init(&ctx, m_h.count());
    crypto_blake2b_update(&ctx, m_h.values(), m_h.count());
    crypto_blake2b_update(&ctx, input, inputSize);
    crypto_blake2b_final(&ctx, m_h.values());
}

void SymmetricState::encryptAndHash(const u8 *input, size_t inputSize, u8 *output) {
    Array<u8, 12> nonce;
    Bytes::WriteLE<u32>(nonce.values(), 0, 0);
    Bytes::WriteLE<u64>(nonce.values(), 4, m_nonce);
    if (input) {
        memcpy(output, input, inputSize);
    }
    u8 *mac = output + inputSize;
    br_poly1305_ctmul_run(m_k.values(), nonce.values(), output, inputSize, m_h.values(),
            m_h.count(), mac, br_chacha20_ct_run, true);
    m_nonce++;
    mixHash(output, inputSize + MACSize);
}

bool SymmetricState::decryptAndHash(const u8 *input, u8 *output, size_t outputSize) {
    Array<u8, 12> nonce;
    Bytes::WriteLE<u32>(nonce.values(), 0, 0);
    Bytes::WriteLE<u64>(nonce.values(), 4, m_nonce);
    if (output) {
        memcpy(output, input, outputSize);
    }
    Array<u8, 16> mac;
    br_poly1305_ctmul_run(m_k.values(), nonce.values(), output, outputSize, m_h.values(),
            m_h.count(), mac.values(), br_chacha20_ct_run, false);
    if (crypto_verify16(mac.values(), input + outputSize)) {
        return false;
    }
    m_nonce++;
    mixHash(input, outputSize + MACSize);
    return true;
}

void SymmetricState::computeSessionKeys(Array<u8, 32> &upstreamK, Array<u8, 32> &downstreamK) {
    HKDFState hkdfState(m_ck, nullptr, 0);
    hkdfState.update(upstreamK.values(), upstreamK.count());
    hkdfState.update(downstreamK.values(), downstreamK.count());
}
