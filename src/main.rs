use clap::{Arg, ArgAction, Command};

use crate::create::create;
use crate::verify::verify;

mod verify;
mod create;

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
		)
		.get_matches();

	let ignored = [
		String::from("sdk_content/**/*.*"),
		String::from("hammer/autosave/*.*"),
		String::from("**/index.csv"),
	];

	if matches.get_flag("new-index") {
		let root = matches.get_one::<String>("root").unwrap();

		let mut excludes = matches.get_many("excluded")
			.map(|it| it.copied().collect())
			.or(Some(Vec::new()))
			.unwrap();

		excludes.push( &ignored[0] );
		excludes.push( &ignored[1] );
		excludes.push( &ignored[2] );

		return create(root, excludes);
	}

	return verify(matches.get_one::<String>("root").unwrap());
}
