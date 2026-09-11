use std::fmt::Display;

use log::error;

pub trait ResultExt {
    fn log_err(self);
}

impl<T, E: Display> ResultExt for Result<T, E> {
    fn log_err(self) {
        if let Err(e) = self {
            error!("{e}");
        }
    }
}
