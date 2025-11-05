use std::net::SocketAddr;

use crate::buffer::Buffer;

pub enum Message {
    Connection { index: usize, addr: SocketAddr, buffer: Buffer }
}

impl Message {
    pub fn index(&self) -> usize {
        match self {
            Message::Connection { index, .. } => *index,
        }
    }
}
