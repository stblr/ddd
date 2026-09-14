use std::fmt::Result;

use crate::storage::Race;
use crate::website::html::Children;
use crate::website::last_race;
use crate::website::pack_link;
use crate::website::page;

pub fn write(race: &Race, page: &mut String) -> Result {
    let write = |body: &mut Children<_>| {
        let mut ul = body.element("ul")?.children()?;
        pack_link::write(&race.pack_hash, "../", &mut ul)?;
        ul.element("li")?.content(race.frame_rate)?;
        last_race::write(race, &mut ul)?;
        ul.finish()
    };

    let name = if race.host_pk.is_some() { "Personal" } else { "Worldwide" };
    page::write(format_args!("{name} Room #{}", race.room_number), write, page)
}
