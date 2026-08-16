use rustls::crypto::CryptoProvider;

mod aead;
mod hash;
mod hmac;
mod sign;
mod verify;

/// Supported key exchange algorithms.
pub mod kx;

/// Supported cipher suites.
pub mod suites;

/// This is a rustls [`CryptoProvider`] using cryptography from Graviola.
///
/// This provides the same algorithms as the rustls *ring*-based
/// provider, which are interoperable and safe defaults for modern TLS.
pub fn default_provider() -> CryptoProvider {
    CryptoProvider {
        cipher_suites: suites::ALL_CIPHER_SUITES.to_vec(),
        kx_groups: kx::ALL_KX_GROUPS.to_vec(),
        signature_verification_algorithms: verify::ALGORITHMS,
        secure_random: &RngProvider,
        key_provider: &sign::Provider,
    }
}

#[derive(Debug)]
struct RngProvider;

impl rustls::crypto::SecureRandom for RngProvider {
    fn fill(&self, bytes: &mut [u8]) -> Result<(), rustls::crypto::GetRandomFailed> {
        graviola::random::fill(bytes).map_err(|_| rustls::crypto::GetRandomFailed)
    }
}

#[cfg(test)]
mod tests {
    use std::io::{Read, Write};
    use std::sync::Arc;

    use rustls::crypto::{CryptoProvider, SupportedKxGroup};
    use rustls::pki_types::pem::PemObject;
    use rustls::pki_types::{CertificateDer, PrivateKeyDer};
    use rustls::sign::CertifiedKey;
    use rustls::{
        ClientConfig, ClientConnection, HandshakeKind, RootCertStore, ServerConfig, ServerConnection,
    };

    use crate::crypto::tls;

    #[test]
    fn all_suites() {
        for key_type in KeyType::ALL {
            test_suite(
                tls::suites::TLS13_CHACHA20_POLY1305_SHA256,
                *key_type,
            );
            test_keys_match(&tls::default_provider(), *key_type);
        }

        for key_type in KeyType::RSA {
            test_suite(
                tls::suites::TLS_ECDHE_RSA_WITH_CHACHA20_POLY1305_SHA256,
                *key_type,
            );
        }

        for key_type in KeyType::ECDSA {
            test_suite(
                tls::suites::TLS_ECDHE_ECDSA_WITH_CHACHA20_POLY1305_SHA256,
                *key_type,
            );
        }
    }

    #[test]
    fn all_key_exchanges() {
        for other in OtherProvider::OTHERS {
            test_key_exchange(tls::kx::X25519MLKEM768, *other, KeyType::Rsa2048);
            test_key_exchange(&tls::kx::X25519, *other, KeyType::Rsa2048);
        }
    }

    fn test_key_exchange(kx: &'static dyn SupportedKxGroup, other: OtherProvider, key_type: KeyType) {
        let provider: Arc<_> = CryptoProvider {
            kx_groups: vec![kx],
            ..tls::default_provider()
        }
        .into();
        test_client(provider.clone(), other, key_type);
        test_server(provider, other, key_type);
    }

    fn test_suite(suite: rustls::SupportedCipherSuite, key_type: KeyType) {
        let provider: Arc<_> = CryptoProvider {
            cipher_suites: vec![suite],
            ..tls::default_provider()
        }
        .into();
        for other in OtherProvider::OTHERS {
            test_client(provider.clone(), *other, key_type);
            test_server(provider.clone(), *other, key_type);
        }
    }

    fn test_client(provider: Arc<CryptoProvider>, other: OtherProvider, key_type: KeyType) {
        let server_config = server_config(other.into_provider(), key_type);
        let client_config = client_config(provider.clone(), key_type);

        assert!(matches!(
            exercise(client_config.clone(), server_config.clone()),
            HandshakeKind::Full | HandshakeKind::FullWithHelloRetryRequest
        ));
        println!("FULL: client with {provider:?} {key_type:?} OK");

        assert_eq!(
            exercise(client_config.clone(), server_config.clone()),
            HandshakeKind::Resumed
        );
        println!("RESUMED: client with {provider:?} {key_type:?} OK");
    }

    fn test_server(provider: Arc<CryptoProvider>, other: OtherProvider, key_type: KeyType) {
        let server_config = server_config(provider.clone(), key_type);
        let client_config = client_config(other.into_provider(), key_type);

        assert!(matches!(
            exercise(client_config.clone(), server_config.clone()),
            HandshakeKind::Full | HandshakeKind::FullWithHelloRetryRequest
        ));
        println!("FULL: server with {provider:?} {key_type:?} OK");

        assert_eq!(
            exercise(client_config, server_config),
            HandshakeKind::Resumed
        );
        println!("RESUMED: server with {provider:?} {key_type:?} OK");
    }

    fn server_config(provider: Arc<CryptoProvider>, key_type: KeyType) -> Arc<ServerConfig> {
        ServerConfig::builder_with_provider(provider)
            .with_safe_default_protocol_versions()
            .unwrap()
            .with_no_client_auth()
            .with_single_cert(key_type.cert_chain(), key_type.key())
            .unwrap()
            .into()
    }

    fn client_config(provider: Arc<CryptoProvider>, key_type: KeyType) -> Arc<ClientConfig> {
        ClientConfig::builder_with_provider(provider)
            .with_safe_default_protocol_versions()
            .unwrap()
            .with_root_certificates(key_type.ca_certs())
            .with_no_client_auth()
            .into()
    }

    fn exercise(client_config: Arc<ClientConfig>, server_config: Arc<ServerConfig>) -> HandshakeKind {
        let mut client = ClientConnection::new(client_config, "localhost".try_into().unwrap()).unwrap();
        let mut server = ServerConnection::new(server_config).unwrap();

        while client.is_handshaking() && server.is_handshaking() {
            let mut buf = [0u8; 1024];
            let wr = client.write_tls(&mut &mut buf[..]).unwrap();
            server.read_tls(&mut &buf[..wr]).unwrap();
            server.process_new_packets().unwrap();

            let wr = server.write_tls(&mut &mut buf[..]).unwrap();
            client.read_tls(&mut &buf[..wr]).unwrap();
            client.process_new_packets().unwrap();
        }

        let _ = client.writer().write(b"hello world").unwrap();
        client.send_close_notify();
        let mut buf = [0u8; 1024];
        let wr = client.write_tls(&mut &mut buf[..]).unwrap();
        server.read_tls(&mut &buf[..wr]).unwrap();
        server.process_new_packets().unwrap();

        let mut out = vec![];
        server.reader().read_to_end(&mut out).unwrap();
        assert_eq!(out, b"hello world");

        let _ = server.writer().write(b"goodbye").unwrap();
        let wr = server.write_tls(&mut &mut buf[..]).unwrap();
        client.read_tls(&mut &buf[..wr]).unwrap();
        client.process_new_packets().unwrap();

        server.handshake_kind().unwrap()
    }

    fn test_keys_match(provider: &CryptoProvider, key_type: KeyType) {
        CertifiedKey::from_der(key_type.cert_chain(), key_type.key(), provider)
            .unwrap()
            .keys_match()
            .unwrap();
    }

    #[derive(Clone, Copy, Debug)]
    enum KeyType {
        Rsa2048,
        Rsa3072,
        Rsa4096,
        EcdsaP256,
        EcdsaP384,
        Ed25519,
    }

    impl KeyType {
        const ALL: &[Self] = &[
            Self::Rsa2048,
            Self::Rsa3072,
            Self::Rsa4096,
            Self::EcdsaP256,
            Self::EcdsaP384,
            Self::Ed25519,
        ];
        const RSA: &[Self] = &[Self::Rsa2048, Self::Rsa3072, Self::Rsa4096];
        const ECDSA: &[Self] = &[Self::EcdsaP256, Self::EcdsaP384];

        fn slug(self) -> &'static str {
            match self {
                Self::Rsa2048 => "rsa-2048",
                Self::Rsa3072 => "rsa-3072",
                Self::Rsa4096 => "rsa-4096",
                Self::EcdsaP256 => "ecdsa-p256",
                Self::EcdsaP384 => "ecdsa-p384",
                Self::Ed25519 => "ed25519",
            }
        }

        fn cert_chain(self) -> Vec<CertificateDer<'static>> {
            CertificateDer::pem_file_iter(format!("../server/src/crypto/tls/keys/{}/end.fullchain", self.slug()))
                .unwrap()
                .collect::<Result<Vec<_>, _>>()
                .unwrap()
        }

        fn key(self) -> PrivateKeyDer<'static> {
            PrivateKeyDer::from_pem_file(format!("../server/src/crypto/tls/keys/{}/end.key", self.slug())).unwrap()
        }

        fn ca_certs(self) -> Arc<RootCertStore> {
            let mut roots = RootCertStore::empty();
            roots
                .add(
                    CertificateDer::from_pem_file(format!("../server/src/crypto/tls/keys/{}/ca.cert", self.slug()))
                        .unwrap(),
                )
                .unwrap();
            roots.into()
        }
    }

    #[derive(Copy, Clone, Debug)]
    enum OtherProvider {
        BaselineGraviola,
        SelfTest,
    }

    impl OtherProvider {
        fn into_provider(self) -> Arc<CryptoProvider> {
            match self {
                Self::BaselineGraviola => rustls_graviola::default_provider().into(),
                Self::SelfTest => tls::default_provider().into(),
            }
        }

        const OTHERS: &[Self] = &[Self::BaselineGraviola];
    }
}
