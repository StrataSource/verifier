use crate::output::{Output, OutputKind};

pub struct CsvOutput;

impl CsvOutput {
	pub fn new() -> Box<CsvOutput> {
		Box::from(Self { })
	}
}

impl Output for CsvOutput {
	fn init(&mut self) {
		println!("type,kind,message,got?,expected?");
	}

	fn write(&mut self, kind: OutputKind, message: String) {
		println!("\"message\",\"{kind}\",\"{message}\"");
	}

	fn report(&mut self, file: &str, message: &str, got: &str, expected: &str) {
		println!("\"report\",\"{file}\",\"{message}\",\"{got}\",\"{expected}\"");
	}
}