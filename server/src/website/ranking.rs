use std::cmp::Reverse;
use std::collections::BTreeMap;
use std::fmt::{Display, Result, Write};
use std::hash::Hash;
use std::ops::{AddAssign, SubAssign};

use jiff::SignedDuration;
use jiff::civil::Date;

use crate::bounds::Bounds;
use crate::sorted::Sorted;
use crate::website::html::Children;
use crate::website::html::Element;
use crate::website::rank::Rank;

#[derive(Debug)]
pub struct Ranking<T, C> {
    counters: BTreeMap<(Date, T), C>,
    recent: Sorted<T, Reverse<C>>,
    total: Sorted<T, Reverse<C>>,
}

impl<T, C: Display> Ranking<T, C> {
    pub fn write<W: Write>(
        &self,
        write_value: impl Fn(&T, Element<W>) -> Result,
        parent: &mut Children<'_, W>,
    ) -> Result {
        write("Last 30 days", &self.recent, &write_value, parent)?;
        write("All-time", &self.total, &write_value, parent)?;
        Ok(())
    }
}

impl<T: Bounds + Copy + Eq + Hash + Ord, C: AddAssign + Copy + Default + Ord + SubAssign>
    Ranking<T, C>
{
    pub fn increment(&mut self, now: Date, date: Date, value: T, amount: C) {
        if now.duration_since(date) < RECENT_DURATION {
            *self.counters.entry((date, value)).or_default() += amount;

            self.recent.modify(value, |counter| counter.0 += amount);
        }

        self.total.modify(value, |counter| counter.0 += amount);
    }

    pub fn update(&mut self, now: Date) {
        let date = now - RECENT_DURATION;
        for ((_, value), amount) in self.counters.extract_if(..=(date, T::MAX), |_, _| true) {
            self.recent.modify(value, |counter| counter.0 -= amount);
        }
    }
}

impl<T, C> Default for Ranking<T, C> {
    fn default() -> Self {
        Self { counters: BTreeMap::default(), recent: Sorted::default(), total: Sorted::default() }
    }
}

fn write<T, W: Write>(
    name: &str,
    sorted: &Sorted<T, Reverse<impl Display>>,
    write_value: impl Fn(&T, Element<W>) -> Result,
    parent: &mut Children<'_, W>,
) -> Result {
    let mut div = parent.element("div")?;
    div.attribute("class")?.value("ranking")?;
    let mut div = div.children()?;

    div.element("h3")?.content(name)?;

    let mut table = div.element("table")?.children()?;
    for (rank, (counter, value)) in sorted.values().iter().enumerate() {
        // TODO limit

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

const RECENT_DURATION: SignedDuration = SignedDuration::from_hours(30 * 24);
