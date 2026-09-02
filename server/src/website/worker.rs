use std::fs;
use std::sync::mpsc::Receiver;

use anyhow::Result;

use crate::storage::Race;
use crate::website::html::Element;

pub struct Worker {
    race_receiver: Receiver<Race>,
    page: String,
}

impl Worker {
    pub fn new(race_receiver: Receiver<Race>) -> Self {
        Self { race_receiver, page: String::new() }
    }

    pub fn run(mut self) {
        self.write_race().unwrap();

        loop {}
    }

    fn write_race(&mut self) -> Result<()> {
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
        head.element("title")?.content("Title")?;
        let mut link = head.element("link")?;
        link.attribute("rel")?.value("stylesheet")?;
        link.attribute("href")?.value("../data/style.css")?;
        link.empty()?;
        head.finish()?;
        let mut body = html.element("body")?.children()?;
        body.element("h1")?.content("Race #12345")?;
        let mut table = body.element("table")?.children()?;
        table.element("caption")?.content("Players")?;
        let mut tr = table.element("tr")?.children()?;
        tr.element("th")?.content("R")?;
        tr.element("th")?.content("P1")?;
        tr.element("th")?.content("P2")?;
        tr.element("th")?.content("Time")?;
        tr.element("th")?.content("Points")?;
        tr.element("th")?.content("C1")?;
        tr.element("th")?.content("C2")?;
        tr.element("th")?.content("Kart")?;
        tr.finish()?;
        for _ in 0..8 {
            let mut tr = table.element("tr")?.children()?;
            tr.element("td")?.content("1st")?;
            let mut td = tr.element("td")?.children()?;
            let mut a = td.element("a")?;
            a.attribute("href")?.value("p0.txt")?;
            a.content("AAA")?;
            td.finish()?;
            let mut td = tr.element("td")?.children()?;
            let mut a = td.element("a")?;
            a.attribute("href")?.value("p1.txt")?;
            a.content("BBB")?;
            td.finish()?;
            tr.element("td")?.content("00:12:829")?;
            tr.element("td")?.content("+999 9999")?;
            tr.element("td")?.content("Petey Piranha")?;
            tr.element("td")?.content("King Boo")?;
            tr.element("td")?.content("Parade Kart")?;
            tr.finish()?;
        }
        table.finish()?;
        body.finish()?;
        html.finish()?;
        eprintln!("<!doctype html>\n{}", self.page);
        fs::write("run/page.html", &self.page)?;
        Ok(())
    }
}
