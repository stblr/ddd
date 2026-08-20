// Based on:
// - https://bearssl.org/gitweb/?p=BearSSL;a=blob;f=src/ec/ecdsa_atr.c
//
// Copyright (c) 2016 Thomas Pornin <pornin@bolet.org>
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
// BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
// ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

fn asn1_to_raw<'a>(
    mut asn1: &[u8],
    raw: &'a mut [u8; 254]
) -> Result<&'a mut [u8], Asn1ToRawError> {
    // Note: this code is a bit lenient in that it accepts a few deviations to DER with regards to
    // minimality of encoding of lengths and integer values. These deviations are still unambiguous.
    //
    // Signature format is a SEQUENCE of two INTEGER values. We support only integers of less than
    // 127 bytes each (signed encoding) so the resulting raw signature will have length at most 254
    // bytes.

    // First byte is SEQUENCE tag.
    if asn1.split_off_first() != Some(&0x30) {
        return Err(Asn1ToRawError);
    }

    // The SEQUENCE length will be encoded over one or two bytes. We limit the total SEQUENCE
    // contents to 255 bytes, because it makes things simpler; this is enough for subgroup orders up
    // to 999 bits.
    let mut z_len = *asn1.split_off_first().ok_or(Asn1ToRawError)? as usize;
    if z_len > 0x80 {
        if z_len != 0x81 {
            return Err(Asn1ToRawError);
        }
        z_len = *asn1.split_off_first().ok_or(Asn1ToRawError)? as usize;
    }
    if z_len != asn1.len() {
        return Err(Asn1ToRawError);
    }

    // First INTEGER (r).
    if asn1.split_off_first() != Some(&0x02) {
        return Err(Asn1ToRawError);
    }
    let r_len = *asn1.split_off_first().ok_or(Asn1ToRawError)? as usize;
    if r_len >= 0x80 {
        return Err(Asn1ToRawError);
    }
    let mut r = asn1.split_off(..r_len).ok_or(Asn1ToRawError)?;

    // Second INTEGER (s).
    if asn1.split_off_first() != Some(&0x02) {
        return Err(Asn1ToRawError);
    }
    let s_len = *asn1.split_off_first().ok_or(Asn1ToRawError)? as usize;
    if s_len >= 0x80 {
        return Err(Asn1ToRawError);
    }
    let mut s = asn1.split_off(..s_len).ok_or(Asn1ToRawError)?;
    if !asn1.is_empty() {
        return Err(Asn1ToRawError);
    }

    // Removing leading zeros from r and s.
    while let Some(new_r) = r.strip_prefix(&[0]) {
        r = new_r;
    }
    while let Some(new_s) = s.strip_prefix(&[0]) {
        s = new_s;
    }

    // Compute common length for the two integers, then copy integers into the raw buffer.
    let z_len = r.len().max(s.len());
    let raw_len = z_len << 1;
    let raw = &mut raw[..raw_len];
    raw.fill(0);
    raw[z_len - r.len()..z_len].copy_from_slice(r);
    raw[raw_len - s.len()..raw_len].copy_from_slice(s);
    Ok(raw)
}

struct Asn1ToRawError;
