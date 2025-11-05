use std::hash::{BuildHasher, RandomState};
use std::net::UdpSocket;
use std::thread;

use anyhow::Result;
use log::error;
use noise_protocol::U8Array;

use crate::crypto::Key;
use crate::formats::online::*;
use crate::message::Message;
use crate::mpsc;
use crate::shard::Shard;

pub fn run(server_k: Key) -> Result<()> {
    let cpu_count = thread::available_parallelism().map_or(1, |cpu_count| cpu_count.get());
    let shard_count = (cpu_count - 1).max(1);
    let socket = UdpSocket::bind(format!("0.0.0.0:{DEFAULT_PORT}"))?;
    let random_state = RandomState::new();

    let mut channels = mpsc::channels(shard_count + 1, 1000);
    for (sender, receiver) in channels.by_ref().take(shard_count) {
        let server_k = server_k.clone();
        let socket = socket.try_clone()?;
        thread::spawn(|| {
            if let Err(e) = Shard::new(server_k, socket, receiver).run() {
                error!("{e}")
            }
        });
    }

    let (sender, receiver) = channels.next().unwrap();
    loop {
        let mut buffer = sender.acquire()?;
        let (len, addr) = socket.recv_from(buffer.as_mut_slice())?;
        let index = random_state.hash_one(addr) as usize % shard_count;
        buffer.set_len(len);
        let message = Message::Connection { index, addr, buffer };
        sender.send(message)?;
    }
}
