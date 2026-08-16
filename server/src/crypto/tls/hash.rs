use orion::hazardous::hash::sha2::{sha256, sha384};
use rustls::crypto::hash::{Context, Hash, HashAlgorithm, Output};

pub struct Sha256;

impl Hash for Sha256 {
    fn start(&self) -> Box<dyn Context> {
        Box::new(Sha256Context(sha256::Sha256::new()))
    }

    fn hash(&self, data: &[u8]) -> Output {
        Output::new(sha256::Sha256::digest(data).unwrap().as_ref())
    }

    fn algorithm(&self) -> HashAlgorithm {
        HashAlgorithm::SHA256
    }

    fn output_len(&self) -> usize {
        sha256::SHA256_OUTSIZE
    }
}

struct Sha256Context(sha256::Sha256);

impl Context for Sha256Context {
    fn fork_finish(&self) -> Output {
        Output::new(self.0.clone().finalize().unwrap().as_ref())
    }

    fn fork(&self) -> Box<dyn Context> {
        Box::new(Self(self.0.clone()))
    }

    fn finish(mut self: Box<Self>) -> Output {
        Output::new(self.0.finalize().unwrap().as_ref())
    }

    fn update(&mut self, data: &[u8]) {
        self.0.update(data).unwrap();
    }
}

pub struct Sha384;

impl Hash for Sha384 {
    fn start(&self) -> Box<dyn Context> {
        Box::new(Sha384Context(sha384::Sha384::new()))
    }

    fn hash(&self, data: &[u8]) -> Output {
        Output::new(sha384::Sha384::digest(data).unwrap().as_ref())
    }

    fn algorithm(&self) -> HashAlgorithm {
        HashAlgorithm::SHA384
    }

    fn output_len(&self) -> usize {
        sha384::SHA384_OUTSIZE
    }
}

struct Sha384Context(sha384::Sha384);

impl Context for Sha384Context {
    fn fork_finish(&self) -> Output {
        Output::new(self.0.clone().finalize().unwrap().as_ref())
    }

    fn fork(&self) -> Box<dyn Context> {
        Box::new(Self(self.0.clone()))
    }

    fn finish(mut self: Box<Self>) -> Output {
        Output::new(self.0.finalize().unwrap().as_ref())
    }

    fn update(&mut self, data: &[u8]) {
        self.0.update(data).unwrap();
    }
}

#[cfg(test)]
mod test {
    use rustls::crypto::hash::Hash;

    use super::*;

    #[test]
    fn test_sha256() {
        let hash = Sha256;
        assert_eq!(hash.algorithm(), HashAlgorithm::SHA256);
        assert_eq!(hash.output_len(), 32);
        let input = b"graviola";
        assert_eq!(hash.hash(input).as_ref(),
            b"\x08\xea\xf2\xeb\x21\x07\x25\xb3\x9f\x46\x3a\x45\x0c\xe9\xe2\xe0\x16\x44\x33\x98\x6a\x08\x70\xf6\x9d\x15\x89\xd4\x55\x7d\x76\xbb"
        );
    }

    #[test]
    fn test_sha384() {
        let hash = Sha384;
        assert_eq!(hash.algorithm(), HashAlgorithm::SHA384);
        assert_eq!(hash.output_len(), 48);
        let input = b"graviola";
        assert_eq!(hash.hash(input).as_ref(),
                   b"\x5e\xbd\x63\x2e\xc3\x17\x2c\x56\x36\x99\x32\x0e\xc9\x38\xb2\x24\x8b\xf6\x97\xa5\x55\x52\xe3\x43\x13\xc4\xce\x5b\x1c\x03\x66\x4f\xcb\x2e\x01\x54\x63\xd1\xdd\x23\x50\x23\x19\xf4\x3a\x30\xc8\xad"
        );
    }
}
