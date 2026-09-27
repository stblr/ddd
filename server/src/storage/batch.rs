use heapless::{LinearMap, Vec};

use crate::crypto::PublicKey;
use crate::formats::online::*;
use crate::room::{Inputs, RaceClient};
use crate::storage::player::Player;
use crate::storage::race::Race;

#[derive(Debug)]
pub struct Batch {
    pub clients: LinearMap<PublicKey, RaceClient, MAX_ROOM_CLIENT_COUNT>,
    pub inputs: Vec<Inputs, MAX_ROOM_KART_COUNT>,
    pub players: Vec<Player, MAX_ROOM_PLAYER_COUNT>,
    pub race: Race,
}
