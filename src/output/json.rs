use std::io::Write;
use crate::output::{Output, OutputKind};

pub struct JsonOutput;

impl JsonOutput {
	pub fn new() -> Box<JsonOutput> {
		Box::from(Self { })
	}
}

impl Output for JsonOutput {
	fn write(&mut self, kind: OutputKind, message: String) {
		println!("{{\"type\":\"message\",\"kind\":\"{kind}\",\"message\":\"{message}\"}}");
		std::io::stdout().flush().unwrap();
	}

	fn report(&mut self, file: &str, message: &str, got: &str, expected: &str) {
		println!("{{\"type\":\"report\",\"file\":\"{file}\",\"message\":\"{message}\",\"got\":\"{got}\",\"expected\":\"{expected}\"}}");
		std::io::stdout().flush().unwrap();
	}
}