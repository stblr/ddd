use std::cmp::Reverse;
use std::collections::HashMap;
use std::fmt::{self, Display, Result, Write};
use std::time::SystemTime;

use jiff::civil::Date;
use jiff::tz::TimeZone;

use crate::courses::Courses;
use crate::formats::online::{CharacterId, KartId, MODE_INDEX_COUNT, ModeIndex};
use crate::player::Name;
use crate::sorted::Sorted;
use crate::storage::Race;
use crate::website::combo::Combo;
use crate::website::counter_ranking::CounterRanking;
use crate::website::course_name;
use crate::website::duration::Duration;
use crate::website::html::{Children, Element};
use crate::website::pack_name;
use crate::website::page;
use crate::website::ranking;

#[derive(Debug, Default)]
pub struct Rankings {
    player_mmrs: [Sorted<u64, Reverse<u16>>; MODE_INDEX_COUNT],
    player_races: CounterRanking<u64, u64>,
    player_times: CounterRanking<u64, Duration>,
    courses: CounterRanking<[u8; 32], u64>,
    packs: CounterRanking<[u8; 32], u64>,
    characters: CounterRanking<CharacterId, u64>,
    karts: CounterRanking<KartId, u64>,
    combos: CounterRanking<Combo, u64>,
}

impl Rankings {
    pub fn add(&mut self, now: Date, race: &Race) {
        let date = race.start.to_zoned(TimeZone::UTC).into();
        self.courses.add(now, date, race.course_hash, 1);
        self.packs.add(now, date, race.pack_hash, 1);
        let start = SystemTime::from(race.start);
        let end = SystemTime::from(race.end);
        let duration = end.duration_since(start).unwrap_or_default();
        let duration = Duration(duration);
        for kart in &race.karts {
            for player in &kart.players {
                self.player_races.add(now, date, player.number, 1);
                self.player_times.add(now, date, player.number, duration);
            }
            for character in kart.characters {
                self.characters.add(now, date, character, 1);
            }
            self.karts.add(now, date, kart.kart, 1);
            let combo = Combo::new(kart.characters, kart.kart);
            self.combos.add(now, date, combo, 1);
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

    pub fn write_players(&self, player_names: &HashMap<u64, Name>, page: &mut String) -> Result {
        write(
            "Player",
            |wv, b| {
                write_ranking("MMR", |d| self.write_player_mmrs(wv, d), b)?;
                write_counter_ranking("Matches", &self.player_races)(wv, b)?;
                write_counter_ranking("Play time", &self.player_times)(wv, b)?;
                Ok(())
            },
            |player, td| {
                let mut td = td.children()?;
                let mut a = td.element("a")?;
                a.attribute("href")?.value(format_args!("../players/{player}"))?;
                let player = fmt::from_fn(|f| {
                    if let Some(name) = player_names.get(player) {
                        write!(f, "{name}")
                    } else {
                        write!(f, "   ")
                    }
                });
                a.content(player)?;
                td.finish()
            },
            page,
        )
    }

    fn write_player_mmrs<W: Write>(
        &self,
        write_value: impl Fn(&u64, Element<W>) -> Result,
        div: &mut Children<W>,
    ) -> Result {
        for mode_index in ModeIndex::VARIANTS {
            ranking::write(mode_index, &self.player_mmrs[mode_index as usize], &write_value, div)?;
        }
        Ok(())
    }

    pub fn write_courses(&self, courses: &Courses, page: &mut String) -> Result {
        write(
            "Course",
            write_counter_ranking("Matches", &self.courses),
            |course, td| td.content(course_name::fmt(courses, course)),
            page,
        )
    }

    pub fn write_packs(&self, page: &mut String) -> Result {
        write(
            "Pack",
            write_counter_ranking("Matches", &self.packs),
            |pack, td| td.content(pack_name::fmt(pack)),
            page,
        )
    }

    pub fn write_characters(&self, page: &mut String) -> Result {
        write(
            "Character",
            write_counter_ranking("Picks", &self.characters),
            |character, td| td.content(character),
            page,
        )
    }

    pub fn write_karts(&self, page: &mut String) -> Result {
        write(
            "Kart",
            write_counter_ranking("Picks", &self.karts),
            |kart, td| td.content(kart),
            page,
        )
    }

    pub fn write_combos(&self, page: &mut String) -> Result {
        write(
            "Combo",
            write_counter_ranking("Picks", &self.combos),
            |combo, td| td.content(combo),
            page,
        )
    }
}

fn write<T, V: Fn(&T, Element<String>) -> Result>(
    name: &str,
    write: impl FnOnce(V, &mut Children<String>) -> Result,
    write_value: V,
    page: &mut String,
) -> Result {
    page::write(format_args!("{name} Rankings"), |body| write(write_value, body), page)
}

fn write_counter_ranking<
    T,
    C: Default + Display + PartialEq,
    V: Fn(&T, Element<W>) -> Result,
    W: Write,
>(
    counter_name: &str,
    ranking: &CounterRanking<T, C>,
) -> impl FnOnce(V, &mut Children<W>) -> Result {
    move |write_value, body| {
        write_ranking(
            counter_name,
            |div| ranking.write(|value, td| write_value(value, td), div),
            body,
        )
    }
}

fn write_ranking<W: Write>(
    counter_name: &str,
    write_ranking: impl FnOnce(&mut Children<W>) -> Result,
    body: &mut Children<W>,
) -> Result {
    body.element("h2")?.content(counter_name)?;
    let mut div = body.element("div")?;
    div.attribute("class")?.value("rankings")?;
    let mut div = div.children()?;
    write_ranking(&mut div)?;
    div.finish()
}
