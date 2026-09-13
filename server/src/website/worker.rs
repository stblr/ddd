use std::collections::HashMap;
use std::fmt::{self, Write};
use std::fs;
use std::path::{Path, PathBuf};
use std::sync::mpsc::Receiver;

use anyhow::Result;
use arc_swap::Cache;
use jiff::Timestamp;
use jiff::tz::TimeZone;

use crate::courses::{Courses, SharedCourses};
use crate::player::Name;
use crate::result_ext::ResultExt;
use crate::storage::{Batch, Player, Race, Stats as StorageStats};
use crate::website::init::Init;
use crate::website::message::Message;
use crate::website::page;
use crate::website::player;
use crate::website::race;
use crate::website::rankings::Rankings;
use crate::website::stats::Stats;

pub struct Worker {
    courses: SharedCourses,
    message_receiver: Receiver<Message>,
    player_names: HashMap<u64, Name>,
    rankings: Rankings,
    stats: Stats,
    races_path: PathBuf,
    players_path: PathBuf,
    rankings_path: PathBuf,
    stats_path: PathBuf,
    page: String,
    file_name_buf: String,
    path_buf: PathBuf,
}

impl Worker {
    pub fn new(
        courses: SharedCourses,
        message_receiver: Receiver<Message>,
        init: Init,
        path: impl AsRef<Path>,
    ) -> Result<Self> {
        let path = path.as_ref().join("website");

        let races_path = path.join("matches");
        fs::create_dir_all(&races_path)?;

        let players_path = path.join("players");
        fs::create_dir_all(&players_path)?;

        let rankings_path = path.join("rankings");
        fs::create_dir_all(&rankings_path)?;

        let stats_path = path.join("stats");
        fs::create_dir_all(&stats_path)?;

        Ok(Self {
            courses,
            message_receiver,
            player_names: init.player_names,
            rankings: init.rankings,
            stats: init.stats,
            races_path,
            players_path,
            rankings_path,
            stats_path,
            page: String::new(),
            file_name_buf: String::new(),
            path_buf: PathBuf::new(),
        })
    }

    pub fn run(mut self) -> ! {
        let mut courses = Cache::new(self.courses.clone());
        loop {
            let mut message = self.message_receiver.recv().unwrap();
            let courses = courses.load();
            match &mut message {
                Message::Batch(batch) => self.process_batch(courses, batch),
                Message::Stats(stats) => self.process_stats(courses, stats),
            }
        }
    }

    fn process_batch(&mut self, courses: &Courses, batch: &mut Batch) {
        self.write_race(courses, &mut batch.race).log_err();
        for player in &batch.players {
            self.write_player(player).log_err();
        }

        for player in &batch.players {
            self.player_names.insert(player.number, player.name);
        }
        let now = Timestamp::now().to_zoned(TimeZone::UTC).into();
        self.rankings.add(now, &batch.race);
        self.stats.add(&batch.race);
    }

    fn process_stats(&mut self, courses: &Courses, stats: &StorageStats) {
        let now = Timestamp::now().to_zoned(TimeZone::UTC).into();
        self.rankings.update(now);
        self.stats.max(stats);
        self.write_rankings(
            |player_names, rankings, page| rankings.write_players(player_names, page),
            "players",
        );
        self.write_rankings(|_, rankings, page| rankings.write_courses(courses, page), "courses");
        self.write_rankings(|_, rankings, page| rankings.write_packs(page), "packs");
        self.write_rankings(|_, rankings, page| rankings.write_characters(page), "characters");
        self.write_rankings(|_, rankings, page| rankings.write_karts(page), "karts");
        self.write_rankings(|_, rankings, page| rankings.write_combos(page), "combos");
        self.write_stat(Stats::write_races, "matches");
        self.write_stat(Stats::write_players, "players");
        self.write_stat(Stats::write_rooms, "rooms");
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

    fn write_stat(
        &mut self,
        write_stat: impl FnOnce(&Stats, &mut String) -> fmt::Result,
        file_name: &str,
    ) {
        || -> Result<()> {
            write_stat(&self.stats, &mut self.page)?;

            self.stats_path.clone_into(&mut self.path_buf);
            self.path_buf.push(file_name);

            self.write_page()
        }()
        .log_err();
    }

    fn write_page(&self) -> Result<()> {
        Ok(fs::write(&self.path_buf, &self.page)?)
    }
}
