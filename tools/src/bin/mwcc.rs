use std::arch::asm;
use std::ffi::{c_uchar, c_uint, c_void};
use std::fs;
use std::mem;
use std::slice;

use anyhow::{Context, Result};
use libc::{MAP_ANON, MAP_FIXED, MAP_PRIVATE, PROT_EXEC, PROT_READ, PROT_WRITE};
use object::pe::{IMAGE_SCN_MEM_EXECUTE, IMAGE_SCN_MEM_READ, IMAGE_SCN_MEM_WRITE};
use object::read::pe::{ImageNtHeaders, PeFile32};
use object::{LittleEndian as LE, Object, ObjectSection};
use syscalls::Sysno;

fn main() -> Result<()> {
    let file = fs::read("tools/cw/modified_mwcceppc.exe")?;
    let file = PeFile32::parse(&*file)?;
    let optional_header = file.nt_headers().optional_header();
    let base = optional_header.image_base.get(LE);
    let size = optional_header.size_of_image.get(LE) as usize;
    let image = unsafe {
        let image = libc::mmap(
            base as *mut c_void,
            size,
            PROT_READ | PROT_WRITE,
            MAP_ANON | MAP_FIXED | MAP_PRIVATE,
            -1,
            0,
        );
        eprintln!("{image:x?}");
        anyhow::ensure!(!image.is_null());
        slice::from_raw_parts_mut(image.cast(), size)
    };
    eprintln!("{:#x?}", image.len());
    for section in file.sections() {
        let address = section.address() as u32;
        let data = section.data()?;
        let start = address.checked_sub(base).context("Invalid section")?;
        let end = start.checked_add(data.len() as u32).context("Invalid section")?;
        let start = start as usize;
        let end = end as usize;
        image.get_mut(start..end).context("Invalid_section")?.copy_from_slice(data);
        eprintln!("{:x?}", (start, end, section.size()));
        let size = section.size() as usize;
        let characteristics = section.pe_section().characteristics.get(LE);
        let mut prot = 0;
        if characteristics & IMAGE_SCN_MEM_READ != 0 {
            eprint!("r");
            prot |= PROT_READ;
        }
        if characteristics & IMAGE_SCN_MEM_WRITE != 0 {
            eprint!("w");
            prot |= PROT_WRITE;
        }
        if characteristics & IMAGE_SCN_MEM_EXECUTE != 0 {
            eprint!("x");
            prot |= PROT_EXEC;
        }
        eprintln!("");
        unsafe {
            anyhow::ensure!(libc::mprotect(address as *mut c_void, size, prot) == 0);
        }
    }
    let stack = unsafe {
        let stack = libc::mmap(
            0xff000000 as *mut c_void,
            0x1000000,
            PROT_READ | PROT_WRITE,
            MAP_ANON | MAP_FIXED | MAP_PRIVATE,
            -1,
            0,
        );
        eprintln!("{stack:x?}");
        anyhow::ensure!(!stack.is_null());
        stack
    };
    let stack = stack as u32;
    let teb = unsafe {
        let teb = libc::mmap(
            0x800000 as *mut c_void,
            0x10000,
            PROT_READ | PROT_WRITE,
            MAP_ANON | MAP_FIXED | MAP_PRIVATE,
            -1,
            0,
        );
        eprintln!("{teb:x?}");
        anyhow::ensure!(!teb.is_null());
        teb
    };
    //let teb = 0x80000 as *mut c_void;
    //unsafe { asm!("wrfsbase {}", in(reg) teb) };
    /*let user_desc = user_desc {
        entry_number: 0,
        base_addr: 0,
        limit: 0xfffff,
        flags: 0b1010101,
    };
    let ptr = &user_desc as *const _;
    let bytecount = mem::size_of_val(&user_desc);
    unsafe {
        eprintln!("{:x?}", ptr);
        eprintln!("{:x?}", bytecount);
        let r = syscalls::syscall!(Sysno::modify_ldt, 1, ptr, bytecount).unwrap_or(!0);
        anyhow::ensure!(r == 0);
    }*/
    /*let user_desc = user_desc {
        entry_number: 1,
        base_addr: 0,
        //base_addr: 0x401000,
        limit: 0x10000,
        //limit: 0x281,
        flags: 0b1010001,
    };
    let ptr = &user_desc as *const _;
    let bytecount = mem::size_of_val(&user_desc);
    unsafe {
        eprintln!("{:x?}", ptr);
        eprintln!("{:x?}", bytecount);
        let r = syscalls::syscall!(Sysno::modify_ldt, 1, ptr, bytecount).unwrap_or(!0);
        anyhow::ensure!(r == 0);
    }*/
    let user_desc = user_desc {
        entry_number: 0,
        base_addr: teb as u32,
        //base_addr: 0x401000,
        limit: 0xfffff,
        //limit: 0x281,
        flags: 0b1010001,
    };
    let ptr = &user_desc as *const _;
    let bytecount = mem::size_of_val(&user_desc);
    unsafe {
        eprintln!("{:x?}", ptr);
        eprintln!("{:x?}", bytecount);
        let r = syscalls::syscall!(Sysno::modify_ldt, 1, ptr, bytecount).unwrap_or(!0);
        anyhow::ensure!(r == 0);
    }
    //unsafe { asm!("ljmp 1, [{}]", in(reg) entry); };
    //unsafe { asm!("wrfsbase {}", in(reg) teb) };
    let entry = file.entry() as u32;
    let far_ptr = FarPtr { offset: entry, sel: 0x23 };
    //let far_ptr = (7 as u64) << 32;
    let far_ptr = &far_ptr as *const _;
    unsafe {
        asm!(
            "mov esp, {:e}",
            "mov ds, {:e}",
            "mov fs, {:e}",
            "jmp fword ptr [{}]",
            in(reg) stack + (0x1000000 - 0x4),
            in(reg) 0x2b,
            in(reg) 0x07,
            in(reg) far_ptr,
        )
    };
    //let entry = file.entry() as usize;
    //let entry: Entry = unsafe { mem::transmute(entry) };
    //eprintln!("{:#x?}", entry);
    //unsafe { entry() };
    Ok(())
}

#[repr(align(64))]
struct Teb([u8; 0x1000]);

#[repr(C)]
struct user_desc {
    entry_number: c_uint,
    base_addr: c_uint,
    limit: c_uint,
    flags: c_uint,
}

#[repr(C)]
struct FarPtr {
    offset: u32,
    sel: u16,
}

type Entry = unsafe extern "C" fn();
