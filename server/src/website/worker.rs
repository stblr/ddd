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
        head.finish()?;
        let mut body = html.element("body")?.children()?;
        let mut table = body.element("table")?.children()?;
        let mut tr = table.element("tr")?.children()?;
        tr.element("th")?.content("R")?;
        tr.element("th")?.content("P")?;
        //tr.element("th")?.content("Combo")?;
        tr.element("th")?.content("Time")?;
        tr.element("th")?.content("Points")?;
        tr.finish()?;
        let mut tr = table.element("tr")?.children()?;
        tr.element("td")?.content("1st")?;
        tr.element("td")?.content("AAA BBB")?;
        //tr.element("td")?.content("Petey Piranha<wbr>King Boo<wbr>Parade Kart")?;
        tr.element("td")?.content("00:12:829")?;
        tr.element("td")?.content("+999&nbsp;9999")?;
        tr.finish()?;
        table.finish()?;
        body.finish()?;
        html.finish()?;
        eprintln!("{}", self.page);
        fs::write("run/page.html", &self.page)?;
        Ok(())
    }
}
