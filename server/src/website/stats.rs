use std::fmt::Result;

use jiff::tz::TimeZone;

use crate::storage::Race;
use crate::website::stat::Stat;

#[derive(Debug, Default)]
pub struct Stats {
    races: Stat,
}

impl Stats {
    pub fn add(&mut self, race: &Race) {
        let dt = race.start.to_zoned(TimeZone::UTC).into();
        self.races.add(dt, 1);
    }

    pub fn write_races(&self, page: &mut String) -> Result {
        self.races.write("Matches", page)
    }
}
