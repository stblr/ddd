#include "Session.hh"

extern "C" {
#include <bearssl/bearssl.h>
}
#include <common/Bytes.hh>
extern "C" {
#include <monocypher/monocypher.h>
}

Session::Session() {
    reset();
}

Session::~Session() {
    m_writeK.fill(0);
    m_readK.fill(0);
}

void Session::reset() {
    m_readK.fill(0);
    m_writeK.fill(0);
    m_readNonce = 0;
    m_writeNonce = 0;
}

bool Session::read(u8 *buffer, u32 size, const u8 mac[MACSize], const u8 nonce[NonceSize]) {
    u64 readNonce = Bytes::ReadLE<u64>(nonce, 0);
    if (readNonce < m_readNonce || readNonce == UINT64_MAX) {
        return false;
    }
    crypto_aead_ctx ctx;
    crypto_aead_init_djb(&ctx, m_readK.values(), nonce);
    if (crypto_aead_read(&ctx, buffer, mac, nullptr, 0, buffer, size)) {
        crypto_wipe(&ctx, sizeof(ctx));
        return false;
    }
    crypto_wipe(&ctx, sizeof(ctx));
    m_readNonce = readNonce + 1;
    return true;
}

void Session::write(u8 *buffer, u32 size, u8 mac[MACSize], u8 nonce[NonceSize]) {
    Array<u8, 12> writeNonce;
    Bytes::WriteLE<u32>(writeNonce.values(), 0, 0);
    Bytes::WriteLE<u64>(writeNonce.values(), 4, m_writeNonce);
    br_poly1305_ctmul_run(m_writeK.values(), writeNonce.values(), buffer, size, nullptr, 0, mac,
            br_chacha20_ct_run, true);
    Bytes::WriteLE<u64>(nonce, 0, m_writeNonce);
    m_writeNonce++;
}
