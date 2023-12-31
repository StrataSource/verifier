pub use crate::output::json::JsonOutput;
pub use crate::output::simple::SimpleOutput;

mod json;
mod simple;

pub enum OutputKind {
	Info,
	Warn,
	Error,
}

impl std::fmt::Display for OutputKind {
	fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
		match self {
			OutputKind::Info => write!(f, "info"),
			OutputKind::Warn => write!(f, "warn"),
			OutputKind::Error => write!(f, "error"),
		}
	}
}

pub trait Output {
	fn init(&mut self) {}
	fn write(&mut self, kind: OutputKind, message: String);
	fn report(&mut self, file: &str, message: &str, got: &str, expected: &str);
	fn end(&mut self) {}
}
