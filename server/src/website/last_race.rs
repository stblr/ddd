use std::fmt::{Result, Write};

use crate::storage::Race;
use crate::website::html::Children;

pub fn write(race: &Race, ul: &mut Children<impl Write>) -> Result {
    let mut li = ul.element("li")?.children()?;
    li.content("Last match:")?;
    let mut a = li.element("a")?;
    a.attribute("href")?.value(format_args!("../matches/{}", race.number))?;
    a.content(format_args!("{} #{}", race.mode, race.number))?;
    li.finish()
}
