use crate::output::{Output, OutputKind};

pub struct SimpleOutput;

impl SimpleOutput {
	pub fn new() -> Box<SimpleOutput> {
		Box::from(Self { })
	}
}

impl Output for SimpleOutput {
	fn write(&mut self, kind: OutputKind, message: String) {
		match kind {
			OutputKind::Info => println!( "Info: {}", message ),
			OutputKind::Warn => eprintln!( "Warn: {}", message ),
			OutputKind::Error => eprintln!( "Error: {}", message ),
		}
	}

	fn report(&mut self, file: &str, message: &str, _got: &str, _expected: &str) {
		eprintln!( "In file `{file}`: {message}" );
	}
}