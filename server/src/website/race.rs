use std::fmt::{self, Result, Write};

use crate::base64;
use crate::courses::Courses;
use crate::storage::race::{Kart, Race};
use crate::website::course_name;
use crate::website::html::Children;
use crate::website::rank::Rank;

pub fn write(courses: &Courses, race: &mut Race, body: &mut Children<impl Write>) -> Result {
    write_ul(courses, race, body)?;
    write_table(race, body)?;
    Ok(())
}

fn write_ul(courses: &Courses, race: &Race, body: &mut Children<impl Write>) -> Result {
    let mut ul = body.element("ul")?.children()?;

    let mut li = ul.element("li")?.children()?;
    let mut a = li.element("a")?;
    a.attribute("href")?.value(format_args!("../rooms/{}", race.room_number))?;
    let name = if race.host_pk.is_some() { "Personal" } else { "Worldwide" };
    a.content(format_args!("{name} Room #{}", race.room_number))?;
    li.finish()?;

    let mut li = ul.element("li")?.children()?;
    li.content("Course: ")?;
    course_name::write(courses, &race.course_hash, &mut li)?;
    li.finish()?;

    let mut li = ul.element("li")?.children()?;
    li.content("Pack: ")?;
    let mut a = li.element("a")?;
    let pack = base64::display(&race.pack_hash);
    a.attribute("href")?.value(format_args!("../packs/{pack}"))?;
    a.content(format_args!("{pack:.12}..."))?;
    li.finish()?;

    ul.element("li")?.content(race.frame_rate)?;

    if let Some(engine_size) = race.engine_size {
        ul.element("li")?.content(engine_size)?;
    }

    if let Some(item_mode) = race.item_mode {
        ul.element("li")?.content(item_mode)?;
    }

    if let Some(lap_count) = race.lap_count
        && lap_count != 0
    {
        let name = if lap_count == 1 { "Lap" } else { "Laps" };
        ul.element("li")?.content(format_args!("{lap_count} {name}"))?;
    }

    let content = if race.host_pk.is_some() {
        format_args!("{}/{}", race.race_index + 1, race.race_count)
    } else {
        format_args!("#{}", race.race_index + 1)
    };
    ul.element("li")?.content(content)?;

    let name = if race.spectator_count == 1 { "Spectator" } else { "Spectators" };
    ul.element("li")?.content(format_args!("{} {name}", race.spectator_count))?;

    ul.element("li")?.content(format_args!("Start: {:.0}", race.start))?;
    ul.element("li")?.content(format_args!("End: {:.0}", race.end))?;

    ul.finish()
}

fn write_table(race: &mut Race, body: &mut Children<impl Write>) -> Result {
    let mut table = body.element("table")?.children()?;
    race.karts.sort_unstable_by_key(|kart| kart.result_index);
    for (rank, kart) in race.karts.iter().enumerate() {
        write_tr(race, rank, kart, &mut table)?;
    }
    table.finish()
}

fn write_tr(race: &Race, rank: usize, kart: &Kart, table: &mut Children<impl Write>) -> Result {
    let mut tr = table.element("tr")?;
    tr.attribute("class")?.value(format_args!("team-{}", kart.team))?;
    let mut tr = tr.children()?;

    tr.element("td")?.content(Rank(rank))?;

    for player in &kart.players {
        let mut td = tr.element("td")?;
        if kart.players.len() == 1 {
            td.attribute("colspan")?.value("2")?;
        }
        let mut td = td.children()?;

        let mut a = td.element("a")?;
        a.attribute("href")?.value(format_args!("../players/{}", player.number))?;
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

    tr.element("td")?.content(format_args!("{} pts", kart.result_points))?;

    for character in kart.characters {
        let mut td = tr.element("td")?.children()?;
        let mut a = td.element("a")?;
        a.attribute("href")?.value(format_args!("../characters/{}", character as u8))?;
        a.content(character)?;
        td.finish()?;
    }

    let mut td = tr.element("td")?.children()?;
    let mut a = td.element("a")?;
    a.attribute("href")?.value(format_args!("../karts/{}", kart.kart as u8))?;
    a.content(kart.kart)?;
    td.finish()?;

    let role = if Some(kart.client_pk) == race.host_pk { "Host" } else { "" };
    tr.element("td")?.content(role)?;

    let mut stat = |title, stat, suffix| -> Result {
        let mut td = tr.element("td")?;
        td.attribute("title")?.value(title)?;
        let stat = fmt::from_fn(move |f| match stat {
            Some(stat) => write!(f, "{stat}{suffix}"),
            None => write!(f, "?"),
        });
        td.content(stat)?;
        Ok(())
    };
    stat("Delayed frames", kart.delayed_frames, "")?;
    stat("Latency", kart.latency, " f")?;
    stat("Stability", kart.stability, "/64")?;

    tr.finish()
}
