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
			.help("Creates a new index file")
			.long("new-index")
			.action(ArgAction::SetTrue)
			.default_value("false")
		)
		.arg(Arg::new("root")
			.help("The engine root directory")
			.long("vproject")
			.action(ArgAction::Set)
			.env("VPROJECT")
			.default_value(".")
		)
		.arg(Arg::new("excluded")
			.help("GLOB pattern(s) to exclude when creating the index")
			.long("exclude")
			.short('e')
			.action(ArgAction::Append)
			.default_values(["sdk_content", "hammer/autosave", "index.csv"])
		)
		.get_matches();

	if matches.get_flag("new-index") {
		return create(
			matches.get_one::<String>("root").unwrap(),
			matches.get_many("excluded").expect("fffff").collect()
		);
	}
	return verify(matches.get_one::<String>("root").unwrap());
}
