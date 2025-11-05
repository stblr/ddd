use std::collections::hash_map::{Entry, HashMap};
use std::hash::{BuildHasher, RandomState};
use std::net::{SocketAddr, UdpSocket};
use std::time::{Duration, Instant};

use anyhow::Result;
use noise_protocol::U8Array;

use crate::client::Client;
use crate::connection::Connection;
use crate::crypto::{Key, PublicKey};
use crate::message::Message;
use crate::mpsc::Receiver;
use crate::rooms::Rooms;

pub struct Shard {
    server_k: Key,
    socket: UdpSocket,
    receiver: Receiver,
    random_state: RandomState,
    connections: HashMap<SocketAddr, (bool, Connection)>,
    clients: HashMap<PublicKey, Client>,
    rooms: Rooms,
    tick_counter: u64,
}

impl Shard {
    pub fn new(server_k: Key, socket: UdpSocket, receiver: Receiver) -> Shard {
        Shard {
            server_k,
            socket,
            receiver,
            random_state: RandomState::new(),
            connections: HashMap::new(),
            clients: HashMap::new(),
            rooms: Rooms::new(),
            tick_counter: 0,
        }
    }

    pub fn run(&mut self) -> Result<()> {
        let mut next_tick = Instant::now();
        loop {
            let now = Instant::now();
            match next_tick.checked_duration_since(now) {
                Some(duration) if !duration.is_zero() => self.read(now, duration)?,
                _ => {
                    self.write(now)?;
                    next_tick += Duration::from_nanos(16_683_333);
                    self.tick_counter += 1;
                }
            }
        }
    }

    fn read(&mut self, now: Instant, duration: Duration) -> Result<()> {
        let Some((index, message)) = self.receiver.recv(duration)? else {
            return Ok(());
        };
        let buffer = match message {
            Message::Connection { addr, buffer, .. } => {
                self.read_connection(now, addr, buffer.as_slice())?;
                buffer
            }
        };
        self.receiver.release(index, buffer)?;
        Ok(())
    }

    fn read_connection(&mut self, now: Instant, addr: SocketAddr, message: &[u8]) -> Result<()> {
        let is_full = self.connections.len() >= 1000;
        match self.connections.entry(addr) {
            Entry::Occupied(mut o) => {
                let (_, connection) = o.get_mut();
                if connection.read(now, message, &mut self.clients).is_err() {
                    o.remove();
                }
            }
            Entry::Vacant(v) if !is_full => {
                let Some((client_cookie, message)) = message.split_first_chunk() else {
                    return Ok(());
                };
                let server_cookie = self.random_state.hash_one((self.tick_counter >> 12, addr));
                let server_cookie = server_cookie.to_be_bytes();
                if *client_cookie != server_cookie {
                    self.socket.send_to(&server_cookie, addr)?;
                    return Ok(());
                }

                let connection = Connection::try_new(self.server_k.clone(), now, addr, message);
                if let Ok(connection) = connection {
                    let retain = true;
                    v.insert((retain, connection));
                }
            }
            _ => (),
        }
        Ok(())
    }

    fn write(&mut self, now: Instant) -> Result<()> {
        self.clients.retain(|_, client| client.update(now, &mut self.rooms).is_ok());
        self.rooms.update(&mut self.clients);
        let player_count = self.clients.values().map(Client::player_count).sum();
        for (addr, (retain, connection)) in &mut self.connections {
            let mut message = [0u8; 512];
            let message_len = connection.write(
                now,
                &mut message,
                &mut self.clients,
                player_count,
                &mut self.rooms,
            );
            let Ok(message_len) = message_len else {
                *retain = false;
                continue;
            };
            let Some(message_len) = message_len else {
                continue;
            };
            let message = &mut message[..message_len];
            self.socket.send_to(message, addr)?;
        }
        self.connections.retain(|_, (retain, _)| *retain);
        Ok(())
    }
}
