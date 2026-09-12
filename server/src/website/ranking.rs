use std::cmp::Reverse;
use std::fmt::{Display, Result, Write};

use crate::sorted::Sorted;
use crate::website::html::{Children, Element};
use crate::website::rank::Rank;

pub fn write<T, W: Write>(
    name: &str,
    sorted: &Sorted<T, Reverse<impl Default + Display + PartialEq>>,
    write_value: impl Fn(&T, Element<W>) -> Result,
    parent: &mut Children<'_, W>,
) -> Result {
    let mut div = parent.element("div")?;
    div.attribute("class")?.value("ranking")?;
    let mut div = div.children()?;

    div.element("h3")?.content(name)?;

    let mut table = div.element("table")?.children()?;
    for (rank, (counter, value)) in sorted.values().iter().enumerate() {
        if rank >= 50 || counter.0 == Default::default() {
            break;
        }

        let mut tr = table.element("tr")?.children()?;

        tr.element("td")?.content(Rank(rank))?;

        let td = tr.element("td")?;
        write_value(value, td)?;

        tr.element("td")?.content(&counter.0)?;

        tr.finish()?;
    }
    table.finish()?;

    div.finish()?;

    Ok(())
}
