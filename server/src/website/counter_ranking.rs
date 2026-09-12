use std::cmp::Reverse;
use std::collections::BTreeMap;
use std::fmt::{Display, Result, Write};
use std::hash::Hash;
use std::ops::{AddAssign, SubAssign};

use jiff::SignedDuration;
use jiff::civil::Date;

use crate::bounds::Bounds;
use crate::sorted::Sorted;
use crate::website::html::{Children, Element};
use crate::website::ranking;

#[derive(Debug)]
pub struct CounterRanking<T, C> {
    counters: BTreeMap<(Date, T), C>,
    recent: Sorted<T, Reverse<C>>,
    total: Sorted<T, Reverse<C>>,
}

impl<T, C: Default + Display + PartialEq> CounterRanking<T, C> {
    pub fn write<W: Write>(
        &self,
        write_value: impl Fn(&T, Element<W>) -> Result,
        parent: &mut Children<'_, W>,
    ) -> Result {
        ranking::write("Last 30 days", &self.recent, &write_value, parent)?;
        ranking::write("All-time", &self.total, &write_value, parent)?;
        Ok(())
    }
}

impl<T: Bounds + Copy + Eq + Hash + Ord, C: AddAssign + Copy + Default + Ord + SubAssign>
    CounterRanking<T, C>
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

impl<T, C> Default for CounterRanking<T, C> {
    fn default() -> Self {
        Self { counters: BTreeMap::default(), recent: Sorted::default(), total: Sorted::default() }
    }
}

const RECENT_DURATION: SignedDuration = SignedDuration::from_hours(30 * 24);
