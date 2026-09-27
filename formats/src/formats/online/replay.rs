use crate::array_type::ArrayType;
use crate::complex_data_type::ComplexDataType;
use crate::formats::online::client_state;
use crate::formats::online::common::*;
use crate::simple_data_type::SimpleDataType;
use crate::struct_type::StructType;

pub fn replay() -> impl ComplexDataType {
    let magic: SimpleDataType<u32> = SimpleDataType::new();
    let protocol_version: SimpleDataType<u16> = SimpleDataType::new();
    let reserved: SimpleDataType<u16> = SimpleDataType::new();
    StructType::new("Replay")
        .with_field("magic", magic)
        .with_field("protocol_version", protocol_version)
        .with_field("reserved", reserved)
}

pub fn replay_race() -> impl ComplexDataType {
    let pack_course_count: SimpleDataType<u8> = SimpleDataType::new();
    let pack_hash_element: SimpleDataType<u8> = SimpleDataType::new();
    let pack_hash = ArrayType::new(pack_hash_element, 32, 32);
    let course_index: SimpleDataType<u8> = SimpleDataType::new();
    let clients = ArrayType::new(replay_client(), 1, MAX_ROOM_CLIENT_COUNT);
    let time: SimpleDataType<u64> = SimpleDataType::new();
    StructType::new("ReplayRace")
        .with_field("frame_rate", frame_rate())
        .with_field("mode_index", mode_index())
        .with_field("pack_course_count", pack_course_count)
        .with_field("pack_hash", pack_hash)
        .with_field("course_index", course_index)
        .with_field("clients", clients)
        .with_field("time", time)
}

pub fn replay_client() -> impl ComplexDataType {
    let pk_element: SimpleDataType<u8> = SimpleDataType::new();
    let pk = ArrayType::new(pk_element, 32, 32);
    let region: SimpleDataType<u8> = SimpleDataType::new();
    let platform_element: SimpleDataType<u8> = SimpleDataType::new();
    let platform = ArrayType::new(platform_element, 0, MAX_PLATFORM_LENGTH);
    let players = ArrayType::new(
        client_state::client_player(),
        MIN_CLIENT_PLAYER_COUNT,
        MAX_CLIENT_PLAYER_COUNT,
    );
    let kart_count: SimpleDataType<u8> = SimpleDataType::new();
    StructType::new("ReplayClient")
        .with_field("pk", pk)
        .with_field("region", region)
        .with_field("platform", platform)
        .with_field("players", players)
        .with_field("kart_count", kart_count)
}

pub fn replay_client_state() -> impl ComplexDataType {
    let server_frame: SimpleDataType<u16> = SimpleDataType::new();
    let input: SimpleDataType<u16> = SimpleDataType::new();
    let inputs = ArrayType::new(input, MIN_CLIENT_PLAYER_COUNT, MAX_CLIENT_PLAYER_COUNT);
    StructType::new("ReplayClientState")
        .with_field("server_frame", server_frame)
        .with_field("inputs", inputs)
}
