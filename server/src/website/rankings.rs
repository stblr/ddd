use std::time::SystemTime;

use jiff::civil::Date;
use jiff::tz::TimeZone;

use crate::formats::online::{CharacterId, KartId};
use crate::storage::Race;
use crate::website::combo::Combo;
use crate::website::counter_ranking::CounterRanking;
use crate::website::duration::Duration;

#[derive(Debug, Default)]
pub struct Rankings {
    player_races: CounterRanking<u64, u64>,
    player_times: CounterRanking<u64, Duration>,
    courses: CounterRanking<[u8; 32], u64>,
    packs: CounterRanking<[u8; 32], u64>,
    characters: CounterRanking<CharacterId, u64>,
    karts: CounterRanking<KartId, u64>,
    combos: CounterRanking<Combo, u64>,
}

impl Rankings {
    pub const fn player_races(&self) -> &CounterRanking<u64, u64> {
        &self.player_races
    }

    pub const fn player_times(&self) -> &CounterRanking<u64, Duration> {
        &self.player_times
    }

    pub const fn courses(&self) -> &CounterRanking<[u8; 32], u64> {
        &self.courses
    }

    pub const fn packs(&self) -> &CounterRanking<[u8; 32], u64> {
        &self.packs
    }

    pub const fn characters(&self) -> &CounterRanking<CharacterId, u64> {
        &self.characters
    }

    pub const fn karts(&self) -> &CounterRanking<KartId, u64> {
        &self.karts
    }

    pub const fn combos(&self) -> &CounterRanking<Combo, u64> {
        &self.combos
    }

    pub fn increment(&mut self, now: Date, race: &Race) {
        let date = race.start.to_zoned(TimeZone::UTC).into();
        self.courses.increment(now, date, race.course_hash, 1);
        self.packs.increment(now, date, race.pack_hash, 1);
        let start = SystemTime::from(race.start);
        let end = SystemTime::from(race.end);
        let duration = end.duration_since(start).unwrap_or_default();
        let duration = Duration(duration);
        for kart in &race.karts {
            for player in &kart.players {
                self.player_races.increment(now, date, player.number, 1);
                self.player_times.increment(now, date, player.number, duration);
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
        self.packs.update(now);
        self.characters.update(now);
        self.karts.update(now);
        self.combos.update(now);
    }
}
