use orion::hazardous::aead::chacha20poly1305::{self, SecretKey};
use rustls::crypto::cipher::{
    AeadKey, InboundOpaqueMessage, InboundPlainMessage, Iv, KeyBlockShape, MessageDecrypter,
    MessageEncrypter, NONCE_LEN, Nonce, OutboundOpaqueMessage, OutboundPlainMessage,
    PrefixedPayload, Tls12AeadAlgorithm, Tls13AeadAlgorithm, UnsupportedOperationError,
    make_tls12_aad, make_tls13_aad,
};
use rustls::{ConnectionTrafficSecrets, ContentType, ProtocolVersion};

pub struct Chacha20Poly1305;

impl Tls13AeadAlgorithm for Chacha20Poly1305 {
    fn encrypter(&self, key: AeadKey, iv: Iv) -> Box<dyn MessageEncrypter> {
        Box::new(ChaChaTls13Cipher(
            SecretKey::from_slice(key.as_ref().try_into().unwrap()).unwrap(),
            iv,
        ))
    }

    fn decrypter(&self, key: AeadKey, iv: Iv) -> Box<dyn MessageDecrypter> {
        Box::new(ChaChaTls13Cipher(
            SecretKey::from_slice(key.as_ref().try_into().unwrap()).unwrap(),
            iv,
        ))
    }

    fn key_len(&self) -> usize {
        32
    }

    fn extract_keys(
        &self,
        key: AeadKey,
        iv: Iv,
    ) -> Result<ConnectionTrafficSecrets, UnsupportedOperationError> {
        Ok(ConnectionTrafficSecrets::Chacha20Poly1305 { key, iv })
    }
}

impl Tls12AeadAlgorithm for Chacha20Poly1305 {
    fn encrypter(&self, key: AeadKey, iv: &[u8], _: &[u8]) -> Box<dyn MessageEncrypter> {
        Box::new(ChaChaTls12Cipher(
            SecretKey::from_slice(key.as_ref().try_into().unwrap()).unwrap(),
            Iv::copy(iv),
        ))
    }

    fn decrypter(&self, key: AeadKey, iv: &[u8]) -> Box<dyn MessageDecrypter> {
        Box::new(ChaChaTls12Cipher(
            SecretKey::from_slice(key.as_ref().try_into().unwrap()).unwrap(),
            Iv::copy(iv),
        ))
    }

    fn key_block_shape(&self) -> KeyBlockShape {
        KeyBlockShape {
            enc_key_len: 32,
            fixed_iv_len: 12,
            explicit_nonce_len: 0,
        }
    }

    fn extract_keys(
        &self,
        key: AeadKey,
        iv: &[u8],
        _explicit: &[u8],
    ) -> Result<ConnectionTrafficSecrets, UnsupportedOperationError> {
        // This should always be true because KeyBlockShape and the Iv nonce len are in agreement.
        debug_assert_eq!(NONCE_LEN, iv.len());
        Ok(ConnectionTrafficSecrets::Chacha20Poly1305 {
            key,
            iv: Iv::new(iv[..].try_into().unwrap()),
        })
    }
}

struct ChaChaTls13Cipher(SecretKey, Iv);

impl MessageEncrypter for ChaChaTls13Cipher {
    fn encrypt(
        &mut self,
        m: OutboundPlainMessage<'_>,
        seq: u64,
    ) -> Result<OutboundOpaqueMessage, rustls::Error> {
        let total_len = self.encrypted_payload_len(m.payload.len());
        let mut payload = PrefixedPayload::with_capacity(total_len);

        payload.extend_from_chunks(&m.payload);
        payload.extend_from_slice(&m.typ.to_array());
        let nonce = Nonce::new(&self.1, seq);
        let aad = make_tls13_aad(total_len);
        let mut tag = [0u8; CHACHAPOLY1305_OVERHEAD];
        payload.extend_from_slice(&tag);

        chacha20poly1305::seal(
            &self.0,
            &nonce.0.into(),
            &payload.as_ref()[..payload.as_ref().len() - tag.len()].to_owned(),
            Some(&aad),
            payload.as_mut(),
        ).unwrap();

        Ok(OutboundOpaqueMessage::new(
            ContentType::ApplicationData,
            ProtocolVersion::TLSv1_2,
            payload,
        ))
    }

    fn encrypted_payload_len(&self, payload_len: usize) -> usize {
        payload_len + 1 + CHACHAPOLY1305_OVERHEAD
    }
}

impl MessageDecrypter for ChaChaTls13Cipher {
    fn decrypt<'a>(
        &mut self,
        mut m: InboundOpaqueMessage<'a>,
        seq: u64,
    ) -> Result<InboundPlainMessage<'a>, rustls::Error> {
        let payload = &mut m.payload;
        let nonce = Nonce::new(&self.1, seq);
        let aad = make_tls13_aad(payload.len());
        if payload.len() < CHACHAPOLY1305_OVERHEAD {
            return Err(rustls::Error::DecryptError);
        }
        let cipher_len = payload.len() - CHACHAPOLY1305_OVERHEAD;

        chacha20poly1305::open(
            &self.0,
            &nonce.0.into(),
            &payload.to_vec(),
            Some(&aad),
            &mut payload[..cipher_len],
        )
        .map_err(|_| rustls::Error::DecryptError)?;

        m.payload.truncate(cipher_len);
        m.into_tls13_unpadded_message()
    }
}

struct ChaChaTls12Cipher(SecretKey, Iv);

impl MessageEncrypter for ChaChaTls12Cipher {
    fn encrypt(
        &mut self,
        m: OutboundPlainMessage<'_>,
        seq: u64,
    ) -> Result<OutboundOpaqueMessage, rustls::Error> {
        let total_len = self.encrypted_payload_len(m.payload.len());
        let mut payload = PrefixedPayload::with_capacity(total_len);

        payload.extend_from_chunks(&m.payload);
        let nonce = Nonce::new(&self.1, seq);
        let aad = make_tls12_aad(seq, m.typ, m.version, m.payload.len());
        let mut tag = [0u8; CHACHAPOLY1305_OVERHEAD];
        payload.extend_from_slice(&tag);

        chacha20poly1305::seal(
            &self.0,
            &nonce.0.into(),
            &m.payload.to_vec(),
            Some(&aad),
            payload.as_mut(),
        ).unwrap();

        Ok(OutboundOpaqueMessage::new(m.typ, m.version, payload))
    }

    fn encrypted_payload_len(&self, payload_len: usize) -> usize {
        payload_len + CHACHAPOLY1305_OVERHEAD
    }
}

impl MessageDecrypter for ChaChaTls12Cipher {
    fn decrypt<'a>(
        &mut self,
        mut m: InboundOpaqueMessage<'a>,
        seq: u64,
    ) -> Result<InboundPlainMessage<'a>, rustls::Error> {
        let payload = &mut m.payload;
        let nonce = Nonce::new(&self.1, seq);
        if payload.len() < CHACHAPOLY1305_OVERHEAD {
            return Err(rustls::Error::DecryptError);
        }
        let cipher_len = payload.len() - CHACHAPOLY1305_OVERHEAD;
        let aad = make_tls12_aad(seq, m.typ, m.version, cipher_len);

        chacha20poly1305::open(
            &self.0,
            &nonce.0.into(),
            &payload.to_vec(),
            Some(&aad),
            &mut payload[..cipher_len],
        )
        .map_err(|_| rustls::Error::DecryptError)?;

        m.payload.truncate(cipher_len);
        Ok(m.into_plain_message())
    }
}

const CHACHAPOLY1305_OVERHEAD: usize = 16;

#[cfg(test)]
mod tests {
    use rustls::Error;

    use super::*;

    fn random_bytes<const L: usize>() -> Result<[u8; L], Error> {
        let mut bytes = [0; L];
        graviola::random::fill(&mut bytes).map_err(|_| rustls::crypto::GetRandomFailed)?;
        Ok(bytes)
    }

    #[test]
    fn test_chacha20_poly1305_tls13() {
        let cipher = Chacha20Poly1305;
        assert_eq!(cipher.key_len(), 32);
        let cipher_key = random_bytes::<32>().unwrap();
        let cipher_iv = random_bytes::<12>().unwrap();
        match rustls::crypto::cipher::Tls13AeadAlgorithm::extract_keys(
            &cipher,
            AeadKey::from(cipher_key),
            Iv::from(cipher_iv),
        )
        .unwrap()
        {
            ConnectionTrafficSecrets::Chacha20Poly1305 { key, iv } => {
                assert_eq!(key.as_ref(), cipher_key);
                assert_eq!(iv.as_ref(), cipher_iv);
            }
            _ => panic!("Unexpected secret type extracted from ChaCha20-Poly1305 cipher"),
        }
    }

    #[test]
    fn test_chacha20_poly1305_tls12() {
        let cipher = Chacha20Poly1305;
        assert_eq!(cipher.key_len(), 32);
        let cipher_key = random_bytes::<32>().unwrap();
        let cipher_iv = random_bytes::<12>().unwrap();
        let unused = [0u8; 1];
        match rustls::crypto::cipher::Tls12AeadAlgorithm::extract_keys(
            &cipher,
            AeadKey::from(cipher_key),
            &cipher_iv,
            &unused,
        )
        .unwrap()
        {
            ConnectionTrafficSecrets::Chacha20Poly1305 { key, iv } => {
                assert_eq!(key.as_ref(), cipher_key);
                assert_eq!(iv.as_ref(), cipher_iv);
            }
            _ => panic!("Unexpected secret type extracted from ChaCha20-Poly1305 cipher"),
        }
    }
}
