use std::ffi::c_void;
use std::fs;
use std::slice;

use anyhow::{Context, Result};
use libc::{MAP_ANON, MAP_PRIVATE, PROT_READ, PROT_WRITE};
use object::read::pe::{ImageNtHeaders, PeFile32};
use object::{LittleEndian as LE, Object, ObjectSection};

fn main() -> Result<()> {
    let file = fs::read("tools/cw/modified_mwcceppc.exe")?;
    let file = PeFile32::parse(&*file)?;
    let optional_header = file.nt_headers().optional_header();
    let base = optional_header.image_base.get(LE);
    let size = optional_header.size_of_image.get(LE) as usize;
    // TODO: remove
    unsafe {
        libc::mmap(base as *mut c_void, size, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0)
    };
    let image = unsafe {
        let image = libc::mmap(
            base as *mut c_void,
            size,
            PROT_READ | PROT_WRITE,
            MAP_ANON | MAP_PRIVATE,
            -1,
            0,
        );
        eprintln!("{image:x?}");
        anyhow::ensure!(!image.is_null());
        slice::from_raw_parts_mut(image.cast(), size)
    };
    eprintln!("{:x?}", image.len());
    for section in file.sections() {
        let address = section.address() as u32;
        let data = section.data()?;
        let start = address.checked_sub(base).context("Invalid section")?;
        let end = start.checked_add(data.len() as u32).context("Invalid section")?;
        let start = start as usize;
        let end = end as usize;
        image.get_mut(start..end).context("Invalid_section")?.copy_from_slice(data);
        eprintln!("{:x?}", (start, end, section.size()));
    }
    let blocks = file.data_directories().relocation_blocks(file.data(), &file.section_table())?;
    for block in blocks.iter().flat_map(|blocks| blocks.into_iter()).take(2) {
        let block = block?;
        eprintln!("{:x?}", block);
        for reloc in block {
            eprintln!("{:x?}", reloc);
        }
    }
    //eprintln!("{:x?}", file.relative_address_base());
    Ok(())
}
