use std::fmt::Write;
use std::fs;
use std::path::{Path, PathBuf};
use std::sync::mpsc::Receiver;

use anyhow::Result;
use log::error;

use crate::storage::Race;
use crate::website::html::Element;

pub struct Worker {
    race_receiver: Receiver<Race>,
    races_path: PathBuf,
    page: String,
    file_name_buf: String,
    path_buf: PathBuf,
}

impl Worker {
    pub fn new(race_receiver: Receiver<Race>, path: impl AsRef<Path>) -> Result<Self> {
        let path = path.as_ref().join("website");

        let races_path = path.join("races");
        fs::create_dir_all(&races_path)?;

        Ok(Self {
            race_receiver,
            races_path,
            page: String::new(),
            file_name_buf: String::new(),
            path_buf: PathBuf::new(),
        })
    }

    pub fn run(mut self) -> ! {
        loop {
            let mut race = self.race_receiver.recv().unwrap();

            if let Err(e) = self.write_race(&mut race) {
                error!("{e}");
            }
        }
    }

    fn write_race(&mut self, race: &mut Race) -> Result<()> {
        "<!doctype html>\n".clone_into(&mut self.page);
        let mut html = Element::new(&mut self.page, 0, "html")?;
        html.attribute("lang")?.value("en-US")?;
        let mut html = html.children()?;
        let mut head = html.element("head")?.children()?;
        let mut meta = head.element("meta")?;
        meta.attribute("name")?.value("viewport")?;
        meta.attribute("content")?.value("width=device-width, initial-scale=1")?;
        meta.empty()?;
        let mut meta = head.element("meta")?;
        meta.attribute("name")?.value("color-scheme")?;
        meta.attribute("content")?.value("light dark")?;
        meta.empty()?;
        let name = if race.mode.is_race() { "Race" } else { "Battle" };
        let title = format_args!("{name} #{} · Double Dash Deluxe", race.number);
        head.element("title")?.content(title)?;
        let mut link = head.element("link")?;
        link.attribute("rel")?.value("stylesheet")?;
        link.attribute("href")?.value("../../../data/style.css")?;
        link.empty()?;
        head.finish()?;
        let mut body = html.element("body")?.children()?;
        body.element("h1")?.content(format_args!("Race #{}", race.number))?;
        let mut table = body.element("table")?.children()?;
        race.karts.sort_unstable_by_key(|kart| kart.result_index);
        for (rank, kart) in race.karts.iter().enumerate() {
            let mut tr = table.element("tr")?;
            tr.attribute("class")?.value(format_args!("team-{}", kart.team))?;
            let mut tr = tr.children()?;

            let rank = match rank {
                0 => "1st",
                1 => "2nd",
                2 => "3rd",
                3 => "4th",
                4 => "5th",
                5 => "6th",
                6 => "7th",
                _ => "8th",
            };
            tr.element("td")?.content(rank)?;

            for player in &kart.players {
                let mut td = tr.element("td")?;
                if kart.players.len() == 1 {
                    td.attribute("colspan")?.value("2")?;
                }
                let mut td = td.children()?;

                let mut a = td.element("a")?;
                a.attribute("href")?.value(format_args!("players/{}", player.number))?;
                a.content(player.name)?;

                td.finish()?;
            }

            let milliseconds = kart.result_time % 1000;
            let seconds = kart.result_time / 1000;
            let minutes = seconds / 60;
            let seconds = seconds % 60;
            let time = format_args!("{minutes:02}:{seconds:02}:{milliseconds:03}");
            tr.element("td")?.content(time)?;

            let point_diff = i32::from(kart.result_points) - i32::from(kart.points);
            tr.element("td")?.content(format_args!("{point_diff:+}"))?;

            tr.element("td")?.content(kart.result_points)?;

            for character in kart.characters {
                let mut td = tr.element("td")?.children()?;
                let mut a = td.element("a")?;
                a.attribute("href")?.value(format_args!("characters/{}", character as u8))?;
                a.content(character)?;
                td.finish()?;
            }

            let mut td = tr.element("td")?.children()?;
            let mut a = td.element("a")?;
            a.attribute("href")?.value(format_args!("kart/{}", kart.kart as u8))?;
            a.content(kart.kart)?;
            td.finish()?;

            tr.finish()?;
        }
        table.finish()?;
        body.finish()?;
        html.finish()?;
        eprintln!("{}", self.page);

        self.file_name_buf.clear();
        write!(self.file_name_buf, "{}", race.number)?;

        self.races_path.clone_into(&mut self.path_buf);
        self.path_buf.push(&self.file_name_buf);

        fs::write(&self.path_buf, &self.page)?;

        Ok(())
    }
}
