use std::fmt::Result;

use crate::formats::online::ModeIndex;
use crate::storage::{Player, Race};
use crate::website::duration::Duration;
use crate::website::html::Children;
use crate::website::last_race;
use crate::website::page;

pub fn write(race: &Race, player: &Player, page: &mut String) -> Result {
    let write = |body: &mut Children<_>| {
        let mut ul = body.element("ul")?.children()?;

        ul.element("li")?.content(format_args!("Index: {}", player.index))?;

        for mode_index in ModeIndex::VARIANTS {
            let Some(mmr) = player.mmrs.get(&mode_index) else { continue };
            ul.element("li")?.content(format_args!("{mode_index} MMR: {mmr}"))?;
        }

        ul.element("li")?.content(format_args!("Matches: {}", player.race_count))?;

        let play_time = Duration(player.play_time);
        ul.element("li")?.content(format_args!("Play time: {play_time}"))?;

        last_race::write(race, &mut ul)?;

        ul.finish()
    };

    page::write(format_args!("{} · Player #{}", player.name, player.number), write, page)
}
