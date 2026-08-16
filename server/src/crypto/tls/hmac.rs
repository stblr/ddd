use orion::hazardous::mac::hmac::sha256::{self, HmacSha256};
use orion::hazardous::mac::hmac::sha384::{self, HmacSha384};
use rustls::crypto::hmac::{Hmac, Key, Tag};

pub struct Sha256Hmac;

impl Hmac for Sha256Hmac {
    fn with_key(&self, key: &[u8]) -> Box<dyn Key> {
        let key = sha256::SecretKey::from_slice(key).unwrap();
        Box::new(Sha256HmacKey(HmacSha256::new(&key)))
    }

    fn hash_output_len(&self) -> usize {
        SHA256_OUTPUT
    }
}

struct Sha256HmacKey(HmacSha256);

impl Key for Sha256HmacKey {
    fn sign_concat(&self, first: &[u8], middle: &[&[u8]], last: &[u8]) -> Tag {
        let mut ctx = self.0.clone();
        ctx.update(first);
        for m in middle {
            ctx.update(m);
        }
        ctx.update(last);
        Tag::new(ctx.finalize().unwrap().unprotected_as_bytes())
    }

    fn tag_len(&self) -> usize {
        SHA256_OUTPUT
    }
}

pub struct Sha384Hmac;

impl Hmac for Sha384Hmac {
    fn with_key(&self, key: &[u8]) -> Box<dyn Key> {
        let key = sha384::SecretKey::from_slice(key).unwrap();
        Box::new(Sha384HmacKey(HmacSha384::new(&key)))
    }

    fn hash_output_len(&self) -> usize {
        SHA384_OUTPUT
    }
}

struct Sha384HmacKey(HmacSha384);

impl Key for Sha384HmacKey {
    fn sign_concat(&self, first: &[u8], middle: &[&[u8]], last: &[u8]) -> Tag {
        let mut ctx = self.0.clone();
        ctx.update(first);
        for m in middle {
            ctx.update(m);
        }
        ctx.update(last);
        Tag::new(ctx.finalize().unwrap().unprotected_as_bytes())
    }

    fn tag_len(&self) -> usize {
        SHA384_OUTPUT
    }
}

const SHA256_OUTPUT: usize = 32;
const SHA384_OUTPUT: usize = 48;
