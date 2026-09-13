use std::fmt::Result;

use jiff::tz::TimeZone;

use crate::storage::{Race, Stats as StorageStats};
use crate::website::stat::Stat;

#[derive(Debug, Default)]
pub struct Stats {
    races: Stat,
    players: Stat,
    rooms: Stat,
}

impl Stats {
    pub fn add(&mut self, race: &Race) {
        let dt = race.start.to_zoned(TimeZone::UTC).into();
        self.races.add(dt, 1);
    }

    pub fn max(&mut self, stats: &StorageStats) {
        self.players.max(stats.dt, stats.player_count);
        self.rooms.max(stats.dt, stats.room_count);
    }

    pub fn write_races(&self, page: &mut String) -> Result {
        self.races.write("Matches", page)
    }

    pub fn write_players(&self, page: &mut String) -> Result {
        self.players.write("Max Players", page)
    }

    pub fn write_rooms(&self, page: &mut String) -> Result {
        self.rooms.write("Max Rooms", page)
    }
}
