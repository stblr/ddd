use std::time::Duration;

use jiff::civil::Date;
use jiff::tz::TimeZone;

use crate::formats::online::{CharacterId, KartId};
use crate::storage::Race;
use crate::website::combo::Combo;
use crate::website::ranking::Ranking;

#[derive(Debug, Default)]
pub struct Rankings {
    player_races: Ranking<u64, u64>,
    player_times: Ranking<u64, Duration>,
    courses: Ranking<[u8; 32], u64>,
    characters: Ranking<CharacterId, u64>,
    karts: Ranking<KartId, u64>,
    combos: Ranking<Combo, u64>,
}

impl Rankings {
    pub const fn player_races(&self) -> &Ranking<u64, u64> {
        &self.player_races
    }

    pub const fn courses(&self) -> &Ranking<[u8; 32], u64> {
        &self.courses
    }

    pub const fn characters(&self) -> &Ranking<CharacterId, u64> {
        &self.characters
    }

    pub const fn karts(&self) -> &Ranking<KartId, u64> {
        &self.karts
    }

    pub const fn combos(&self) -> &Ranking<Combo, u64> {
        &self.combos
    }

    pub fn increment(&mut self, now: Date, race: &Race) {
        let date = race.start.to_zoned(TimeZone::UTC).into();
        self.courses.increment(now, date, race.course_hash, 1);
        for kart in &race.karts {
            for player in &kart.players {
                self.player_races.increment(now, date, player.number, 1);
            }
            for character in kart.characters {
                self.characters.increment(now, date, character, 1);
            }
            self.karts.increment(now, date, kart.kart, 1);
            let combo = Combo::new(kart.characters, kart.kart);
            self.combos.increment(now, date, combo, 1);
        }
    }

    pub fn update(&mut self, now: Date) {
        self.player_races.update(now);
        self.player_times.update(now);
        self.courses.update(now);
        self.characters.update(now);
        self.karts.update(now);
        self.combos.update(now);
    }
}
