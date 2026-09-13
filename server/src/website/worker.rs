use std::collections::HashMap;
use std::fmt::{self, Write};
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
use crate::website::init::Init;
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
                |player_names, rankings, page| rankings.write_players(player_names, page),
                "players",
            );
            self.write_rankings(
                |_, rankings, page| rankings.write_courses(courses, page),
                "courses",
            );
            self.write_rankings(|_, rankings, page| rankings.write_packs(page), "packs");
            self.write_rankings(|_, rankings, page| rankings.write_characters(page), "characters");
            self.write_rankings(|_, rankings, page| rankings.write_karts(page), "karts");
            self.write_rankings(|_, rankings, page| rankings.write_combos(page), "combos");
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

    fn write_rankings(
        &mut self,
        write_rankings: impl FnOnce(&HashMap<u64, Name>, &Rankings, &mut String) -> fmt::Result,
        file_name: &str,
    ) {
        || -> Result<()> {
            write_rankings(&self.player_names, &self.rankings, &mut self.page)?;

            self.rankings_path.clone_into(&mut self.path_buf);
            self.path_buf.push(file_name);

            self.write_page()
        }()
        .log_err();
    }

    fn write_page(&self) -> Result<()> {
        Ok(fs::write(&self.path_buf, &self.page)?)
    }
}
