use std::fmt::{Result, Write};

use crate::formats::online::ModeIndex;
use crate::storage::Player;
use crate::website::duration::Duration;
use crate::website::html::Children;

pub fn write(player: &Player, body: &mut Children<impl Write>) -> Result {
    let mut ul = body.element("ul")?.children()?;

    ul.element("li")?.content(format_args!("Index: {}", player.index))?;

    for mode_index in ModeIndex::VARIANTS {
        let Some(mmr) = player.mmrs.get(&mode_index) else { continue };
        ul.element("li")?.content(format_args!("{mode_index} MMR: {mmr}"))?;
    }

    ul.element("li")?.content(format_args!("Matches: {}", player.race_count))?;

    let play_time = Duration(player.play_time);
    ul.element("li")?.content(format_args!("Play time: {play_time}"))?;

    ul.finish()
}
