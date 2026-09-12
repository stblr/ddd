use std::collections::HashMap;

use crate::player::Name;
use crate::website::rankings::Rankings;

#[derive(Debug, Default)]
pub struct Init {
    pub player_names: HashMap<u64, Name>,
    pub rankings: Rankings,
}
