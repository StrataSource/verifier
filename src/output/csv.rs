use std::io::Write;
use crate::output::{Output, OutputKind};

pub struct CsvOutput;

impl CsvOutput {
	pub fn new() -> Box<CsvOutput> {
		Box::from(Self { })
	}
}

impl Output for CsvOutput {
	fn init(&mut self) {
		println!("type,context,message,got?,expected?");
	}

	fn write(&mut self, kind: OutputKind, message: String) {
		println!("\"message\",\"{kind}\",\"{message}\"");
		std::io::stdout().flush().unwrap();
	}

	fn report(&mut self, file: &str, message: &str, got: &str, expected: &str) {
		println!("\"report\",\"{file}\",\"{message}\",\"{got}\",\"{expected}\"");
		std::io::stdout().flush().unwrap();
	}
}