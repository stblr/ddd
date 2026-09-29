use std::time::SystemTime;

use crate::formats::online::*;
use crate::storage::race::Race;

pub fn write(race: &Race, buf: &mut Vec<u8>) {
    let mut message = [0u8; BUFFER_SIZE as usize];

    let replay = Replay { magic: REPLAY_MAGIC, protocol_version: PROTOCOL_VERSION, reserved: 0 };
    write_message(buf, &mut message, &replay, Replay::write);

    let mut clients = heapless::Vec::new();
    let mut client_pk = None;
    for kart in &race.karts {
        if Some(kart.client_pk) != client_pk {
            clients
                .push(ReplayClient {
                    pk: kart.client_pk,
                    region: kart.region.into(),
                    platform: kart.platform.clone().into(),
                    players: heapless::Vec::new(),
                    teams: heapless::Vec::new(),
                })
                .unwrap();
            client_pk = Some(kart.client_pk);
        }
        let client = clients.last_mut().unwrap();
        for player in &kart.players {
            client
                .players
                .push(ClientPlayer { profile: player.profile, name: player.name.0 })
                .unwrap();
        }
        client.teams.push(kart.team).unwrap();
    }
    let time = SystemTime::from(race.end)
        .duration_since(SystemTime::UNIX_EPOCH)
        .unwrap_or_default()
        .as_secs();
    let replay_race = ReplayRace {
        frame_rate: race.frame_rate,
        mode_index: race.mode,
        pack_course_count: race.pack_course_count as u8,
        pack_hash: race.pack_hash,
        course_index: race.karts[race.selected_kart_index as usize].course_index,
        clients,
        time,
    };
    write_message(buf, &mut message, &replay_race, ReplayRace::write);
}

fn write_message<T>(
    buf: &mut Vec<u8>,
    message: &mut [u8],
    x: &T,
    write: for<'a> fn(&T, &'a mut [u8]) -> Result<&'a mut [u8], ()>,
) {
    let message_len = message.len() - write(x, message).unwrap().len();
    buf.extend(&message[..message_len]);
}
