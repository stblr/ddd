#include "DH.hh"

#include "payload/crypto/Random.hh"

namespace DH {

DH::K::K() {
    br_ec_keygen(Random::Ctx(), &br_ec_c25519_m31, &m_k, m_x.values(), BR_EC_curve25519);
}

DH::K::~K() {
    m_x.fill(0);
}

DH::PK::PK() {
    m_pk.curve = BR_EC_curve25519;
    m_pk.q = m_q.values();
    m_pk.qlen = m_q.count();
}

DH::PK::PK(const K &k) {
    br_ec_compute_pub(&br_ec_c25519_m31, &m_pk, m_q.values(), &k.m_k);
}

DH::PK::~PK() {}

DH::SK::SK(const K &k, const PK &pk) {
    m_g = pk.m_q;
    br_ec_c25519_m31.mul(m_g.values(), m_g.count(), k.m_x.values(), k.m_x.count(),
            BR_EC_curve25519);
}

DH::SK::~SK() {
    m_g.fill(0);
}

} // namespace DH
