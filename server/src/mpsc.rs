use std::sync::mpsc::{self, RecvTimeoutError, SyncSender};
use std::time::Duration;

use anyhow::Result;

use crate::buffer::Buffer;
use crate::message::Message;

pub fn channels(count: usize, bound: usize) -> impl Iterator<Item = (Sender, Receiver)> {
    let (used_senders, used_receivers): (Vec<_>, Vec<_>) =
        (0..count).map(|_| mpsc::sync_channel(bound)).unzip();
    let (free_senders, free_receivers): (Vec<_>, Vec<_>) =
        (0..count).map(|_| mpsc::sync_channel(bound)).unzip();

    for free_sender in &free_senders {
        for _ in 0..bound {
            let buffer = Buffer::new();
            free_sender.send(buffer).unwrap();
        }
    }

    used_receivers.into_iter().zip(free_receivers).enumerate().map(
        move |(index, (used_receiver, free_receiver))| {
            let used_senders = used_senders.clone();
            let free_senders = free_senders.clone();

            let sender = Sender { index, free_receiver, used_senders };
            let receiver = Receiver { used_receiver, free_senders };

            (sender, receiver)
        },
    )
}

pub struct Sender {
    index: usize,
    free_receiver: mpsc::Receiver<Buffer>,
    used_senders: Vec<SyncSender<(usize, Message)>>,
}

impl Sender {
    pub fn acquire(&self) -> Result<Buffer> {
        let mut buffer = self.free_receiver.recv()?;
        buffer.reset_len();
        Ok(buffer)
    }

    pub fn send(&self, message: Message) -> Result<()> {
        Ok(self.used_senders[message.index()].send((self.index, message))?)
    }
}

pub struct Receiver {
    used_receiver: mpsc::Receiver<(usize, Message)>,
    free_senders: Vec<SyncSender<Buffer>>,
}

impl Receiver {
    pub fn recv(&self, timeout: Duration) -> Result<Option<(usize, Message)>> {
        match self.used_receiver.recv_timeout(timeout) {
            Err(RecvTimeoutError::Timeout) => Ok(None),
            r => Ok(Some(r?)),
        }
    }

    pub fn release(&self, index: usize, buffer: Buffer) -> Result<()> {
        Ok(self.free_senders[index].send(buffer)?)
    }
}
