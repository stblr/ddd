use std::collections::HashMap;
use std::fmt::{self, Display, Write};
use std::fs;
use std::path::{Path, PathBuf};
use std::sync::mpsc::{Receiver, RecvTimeoutError};
use std::time::{Duration, Instant};

use anyhow::Result;
use arc_swap::Cache;
use jiff::Timestamp;
use jiff::tz::TimeZone;

use crate::courses::{Courses, SharedCourses};
use crate::player::Name;
use crate::result_ext::ResultExt;
use crate::storage::{Batch, Player, Race};
use crate::website::counter_ranking::CounterRanking;
use crate::website::course_name;
use crate::website::html::{Children, Element};
use crate::website::init::Init;
use crate::website::pack_name;
use crate::website::page;
use crate::website::player;
use crate::website::race;
use crate::website::rankings::Rankings;

pub struct Worker {
    courses: SharedCourses,
    batch_receiver: Receiver<Batch>,
    player_names: HashMap<u64, Name>,
    rankings: Rankings,
    races_path: PathBuf,
    players_path: PathBuf,
    rankings_path: PathBuf,
    page: String,
    file_name_buf: String,
    path_buf: PathBuf,
}

impl Worker {
    pub fn new(
        courses: SharedCourses,
        batch_receiver: Receiver<Batch>,
        init: Init,
        path: impl AsRef<Path>,
    ) -> Result<Self> {
        let path = path.as_ref().join("website");

        let races_path = path.join("races");
        fs::create_dir_all(&races_path)?;

        let players_path = path.join("players");
        fs::create_dir_all(&players_path)?;

        let rankings_path = path.join("rankings");
        fs::create_dir_all(&rankings_path)?;

        Ok(Self {
            courses,
            batch_receiver,
            player_names: init.player_names,
            rankings: init.rankings,
            races_path,
            players_path,
            rankings_path,
            page: String::new(),
            file_name_buf: String::new(),
            path_buf: PathBuf::new(),
        })
    }

    pub fn run(mut self) -> ! {
        let mut courses = Cache::new(self.courses.clone());
        let mut next_tick = Instant::now();
        loop {
            let courses = courses.load();
            let now = Instant::now();
            if let Some(duration) = next_tick.checked_duration_since(now)
                && !duration.is_zero()
            {
                let mut batch = match self.batch_receiver.recv_timeout(duration) {
                    Err(RecvTimeoutError::Timeout) => continue,
                    batch => batch.unwrap(),
                };
                self.write_race(courses, &mut batch.race).log_err();
                for player in &batch.players {
                    self.write_player(player).log_err();
                }

                let now = Timestamp::now().to_zoned(TimeZone::UTC).into();
                self.rankings.increment(now, &batch.race);
                for player in &batch.players {
                    self.player_names.insert(player.number, player.name);
                }

                continue;
            }

            let now = Timestamp::now().to_zoned(TimeZone::UTC).into();
            self.rankings.update(now);
            self.write_rankings(
                "Player",
                |pn, r, wv, b| {
                    write_counter_ranking("Matches", Rankings::player_races)(pn, r, wv, b)?;
                    write_counter_ranking("Play time", Rankings::player_times)(pn, r, wv, b)?;
                    Ok(())
                },
                |player_names, player, td| {
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
                "players",
            )
            .log_err();
            self.write_rankings(
                "Course",
                write_counter_ranking("Matches", Rankings::courses),
                |_, course, td| td.content(course_name::fmt(courses, course)),
                "courses",
            )
            .log_err();
            self.write_rankings(
                "Pack",
                write_counter_ranking("Matches", Rankings::packs),
                |_, pack, td| td.content(pack_name::fmt(pack)),
                "packs",
            )
            .log_err();
            self.write_rankings(
                "Character",
                write_counter_ranking("Picks", Rankings::characters),
                |_, character, td| td.content(character),
                "characters",
            )
            .log_err();
            self.write_rankings(
                "Kart",
                write_counter_ranking("Picks", Rankings::karts),
                |_, kart, td| td.content(kart),
                "karts",
            )
            .log_err();
            self.write_rankings(
                "Combo",
                write_counter_ranking("Picks", Rankings::combos),
                |_, combo, td| td.content(combo),
                "combos",
            )
            .log_err();
            next_tick += Duration::from_secs(60);
        }
    }

    fn write_race(&mut self, courses: &Courses, race: &mut Race) -> Result<()> {
        let (mode, number) = (race.mode, race.number);
        page::write(
            format_args!("{mode} #{number}"),
            |body| race::write(courses, race, body),
            &mut self.page,
        )?;

        self.file_name_buf.clear();
        write!(self.file_name_buf, "{}", race.number)?;

        self.races_path.clone_into(&mut self.path_buf);
        self.path_buf.push(&self.file_name_buf);

        self.write_page()
    }

    fn write_player(&mut self, player: &Player) -> Result<()> {
        page::write(
            format_args!("{} · Player #{}", player.name, player.number),
            |body| player::write(player, body),
            &mut self.page,
        )?;

        self.file_name_buf.clear();
        write!(self.file_name_buf, "{}", player.number)?;

        self.players_path.clone_into(&mut self.path_buf);
        self.path_buf.push(&self.file_name_buf);

        self.write_page()
    }

    fn write_rankings<T, V: Fn(&HashMap<u64, Name>, &T, Element<String>) -> fmt::Result>(
        &mut self,
        name: &str,
        write_ranking: impl FnOnce(
            &HashMap<u64, Name>,
            &Rankings,
            V,
            &mut Children<String>,
        ) -> fmt::Result,
        write_value: V,
        file_name: &str,
    ) -> Result<()> {
        page::write(
            format_args!("{name} Rankings"),
            |body| write_ranking(&self.player_names, &self.rankings, write_value, body),
            &mut self.page,
        )?;

        self.rankings_path.clone_into(&mut self.path_buf);
        self.path_buf.push(file_name);

        self.write_page()
    }

    fn write_page(&self) -> Result<()> {
        Ok(fs::write(&self.path_buf, &self.page)?)
    }
}

fn write_counter_ranking<
    T,
    C: Default + Display + PartialEq,
    V: Fn(&HashMap<u64, Name>, &T, Element<W>) -> fmt::Result,
    W: Write,
>(
    counter_name: &str,
    ranking: impl Fn(&Rankings) -> &CounterRanking<T, C>,
) -> impl FnOnce(&HashMap<u64, Name>, &Rankings, V, &mut Children<W>) -> fmt::Result {
    move |player_names, rankings, write_value, body| {
        write_ranking(
            counter_name,
            |div| ranking(rankings).write(|value, td| write_value(player_names, value, td), div),
            body,
        )
    }
}

fn write_ranking<W: Write>(
    counter_name: &str,
    write_ranking: impl FnOnce(&mut Children<W>) -> fmt::Result,
    body: &mut Children<W>,
) -> fmt::Result {
    body.element("h2")?.content(counter_name)?;
    let mut div = body.element("div")?;
    div.attribute("class")?.value("rankings")?;
    let mut div = div.children()?;
    write_ranking(&mut div)?;
    div.finish()
}
