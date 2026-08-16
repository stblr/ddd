use orion::hazardous::ecc::x25519::{self, PrivateKey, PublicKey};
use rand::TryRng;
use rand::rngs::SysRng;
use rustls::{Error, NamedGroup, PeerMisbehaved};
use rustls::crypto::{ActiveKeyExchange, GetRandomFailed, SharedSecret, SupportedKxGroup};
use rustls::ffdhe_groups::FfdheGroup;

mod hybrid;
mod mlkem;

/// All key exchange algorithms, in order of preference.
pub static ALL_KX_GROUPS: &[&dyn SupportedKxGroup] = &[
    X25519MLKEM768,
    &X25519 as &dyn SupportedKxGroup,
];

/// Key exchange using X25519.
#[derive(Debug)]
pub struct X25519;

impl SupportedKxGroup for X25519 {
    fn start(&self) -> Result<Box<dyn ActiveKeyExchange>, Error> {
        let priv_key = PrivateKey::generate();
        let pub_key = PublicKey::try_from(&priv_key).unwrap();
        let pub_key_bytes = pub_key.to_bytes();

        Ok(Box::new(ActiveX25519 {
            pub_key_bytes,
            priv_key,
        }))
    }

    fn ffdhe_group(&self) -> Option<FfdheGroup<'static>> {
        None
    }

    fn name(&self) -> NamedGroup {
        NamedGroup::X25519
    }
}

struct ActiveX25519 {
    priv_key: PrivateKey,
    pub_key_bytes: [u8; 32],
}

impl ActiveKeyExchange for ActiveX25519 {
    fn complete(self: Box<Self>, peer: &[u8]) -> Result<SharedSecret, Error> {
        let shared_secret = PublicKey::from_slice(peer)
            .and_then(|their_pub| x25519::key_agreement(&self.priv_key, &their_pub))
            .map_err(|_| Error::from(PeerMisbehaved::InvalidKeyShare))?;
        Ok(SharedSecret::from(shared_secret.unprotected_as_bytes()))
    }

    fn pub_key(&self) -> &[u8] {
        &self.pub_key_bytes
    }

    fn ffdhe_group(&self) -> Option<FfdheGroup<'static>> {
        None
    }

    fn group(&self) -> NamedGroup {
        X25519.name()
    }
}

/// Hybrid key exchange using X25519 and ML-KEM-768.
pub static X25519MLKEM768: &dyn SupportedKxGroup = &hybrid::Hybrid {
    classical: &X25519,
    post_quantum: &mlkem::MlKem768,
    layout: hybrid::Layout {
        classical_share_len: 32,
        post_quantum_first: true,
        post_quantum_client_share_len: mlkem::MlKem768::ENCAPS_LEN,
        post_quantum_server_share_len: mlkem::MlKem768::CIPHERTEXT_LEN,
    },
    name: NamedGroup::X25519MLKEM768,
};

#[cfg(test)]
mod tests {
    use rustls::ProtocolVersion;

    use super::*;

    #[test]
    fn test_kx_x25519() {
        // Create a private key and verify its metadata.
        let key = X25519;
        assert_eq!(key.name(), NamedGroup::X25519);
        assert_eq!(key.ffdhe_group(), None);
        assert!(key.usable_for_version(ProtocolVersion::TLSv1_2));
        assert!(key.usable_for_version(ProtocolVersion::TLSv1_3));

        // A key exchange with an invalid peer public key should fail.
        let active = key.start().unwrap();
        assert!(active.complete(&[0u8]).is_err());

        // A key exchange with a valid peer public key should succeed.
        let active = key.start().unwrap();
        assert_eq!(active.ffdhe_group(), None);
        let peer = PrivateKey::generate();
        let peer_public_key = PublicKey::try_from(&peer).unwrap().to_bytes();
        assert!(active.complete(&peer_public_key).is_ok());
    }
}
