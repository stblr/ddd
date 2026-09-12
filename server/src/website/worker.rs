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
use crate::result_ext::ResultExt;
use crate::storage::{Batch, Race};
use crate::website::Rankings;
use crate::website::course_name;
use crate::website::html::Element;
use crate::website::page;
use crate::website::race;
use crate::website::ranking::Ranking;

pub struct Worker {
    courses: SharedCourses,
    batch_receiver: Receiver<Batch>,
    rankings: Rankings,
    races_path: PathBuf,
    rankings_path: PathBuf,
    page: String,
    file_name_buf: String,
    path_buf: PathBuf,
}

impl Worker {
    pub fn new(
        courses: SharedCourses,
        batch_receiver: Receiver<Batch>,
        rankings: Rankings,
        path: impl AsRef<Path>,
    ) -> Result<Self> {
        let path = path.as_ref().join("website");

        let races_path = path.join("races");
        fs::create_dir_all(&races_path)?;

        let rankings_path = path.join("rankings");
        fs::create_dir_all(&rankings_path)?;

        Ok(Self {
            courses,
            batch_receiver,
            rankings,
            races_path,
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
                continue;
            }

            let now = Timestamp::now().to_zoned(TimeZone::UTC).into();
            self.rankings.update(now);
            self.write_ranking(
                "Players",
                "Matches",
                Rankings::player_races,
                |player, td| td.content(player),
                "players",
            )
            .log_err();
            self.write_ranking(
                "Course",
                "Matches",
                Rankings::courses,
                |course, td| {
                    let mut td = td.children()?;
                    course_name::write(courses, course, &mut td)?;
                    td.finish()
                },
                "courses",
            )
            .log_err();
            self.write_ranking(
                "Character",
                "Picks",
                Rankings::characters,
                |character, td| td.content(character),
                "characters",
            )
            .log_err();
            self.write_ranking(
                "Kart",
                "Picks",
                Rankings::karts,
                |kart, td| td.content(kart),
                "karts",
            )
            .log_err();
            self.write_ranking(
                "Combo",
                "Picks",
                Rankings::combos,
                |combo, td| td.content(combo),
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

        fs::write(&self.path_buf, &self.page)?;

        Ok(())
    }

    fn write_ranking<T, C: Display>(
        &mut self,
        name: &str,
        counter_name: &str,
        ranking: impl Fn(&Rankings) -> &Ranking<T, C>,
        write_value: impl Fn(&T, Element<String>) -> fmt::Result,
        file_name: &str,
    ) -> Result<()> {
        page::write(
            format_args!("{name} Rankings"),
            |body| {
                body.element("h2")?.content(counter_name)?;
                let mut div = body.element("div")?;
                div.attribute("class")?.value("rankings")?;
                let mut div = div.children()?;
                ranking(&self.rankings).write(write_value, &mut div)?;
                div.finish()
            },
            &mut self.page,
        )?;

        self.rankings_path.clone_into(&mut self.path_buf);
        self.path_buf.push(file_name);

        fs::write(&self.path_buf, &self.page)?;

        Ok(())
    }
}
