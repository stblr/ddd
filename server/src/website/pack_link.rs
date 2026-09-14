use std::fmt::{Result, Write};

use crate::website::html::Children;
use crate::website::pack_name;

pub fn write(pack_hash: &[u8; 32], prefix: &str, ul: &mut Children<impl Write>) -> Result {
    let mut li = ul.element("li")?.children()?;
    li.content("Pack:")?;
    let mut a = li.element("a")?;
    a.attribute("href")?.value(format_args!("{prefix}rankings/packs"))?;
    a.content(pack_name::fmt(pack_hash))?;
    li.finish()
}
