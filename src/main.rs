mod verify;
mod create;

use clap::{Arg, ArgAction, Command};

use crate::create::create;
use crate::verify::verify;

fn main() {
	let matches = Command::new("install_checker")
		.author("ENDERZOMBI102 <enderzombi102.end@gmail.com>")
		.version("0.1.0")
		.about("Used to verify a game's install")
		.arg(Arg::new("new-index")
			.long("new-index")
			.action(ArgAction::SetTrue)
			.default_value("false")
		)
		.arg(Arg::new("root")
			.long("vproject")
			.action(ArgAction::Set)
			.env("VPROJECT")
			.default_value(".")
		)
		.get_matches();

	if matches.get_flag("new-index") {
		return create(matches.get_one::<String>("root").unwrap());
	}
	return verify(matches.get_one::<String>("root").unwrap());
}
