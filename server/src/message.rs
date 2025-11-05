use std::net::SocketAddr;

use crate::buffer::Buffer;

pub enum Message {
    Connection { index: usize, addr: SocketAddr, buffer: Buffer },
    Client { index: usize, buffer: Buffer },
}

impl Message {
    pub fn index(&self) -> usize {
        match self {
            Message::Connection { index, .. } => *index,
            Message::Client { index, .. } => *index,
        }
    }
}
