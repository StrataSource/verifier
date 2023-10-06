use clap::{Arg, ArgAction, Command};
use clap::builder::PossibleValuesParser;

use crate::create::create;
use crate::output::{CsvOutput, JsonOutput, Output, SimpleOutput};
use crate::verify::verify;

mod verify;
mod create;
mod output;

// correct the index path with os
#[cfg(target_os = "windows")]
const INDEX_PATH: &str = "bin/win64/index.csv";
#[cfg(target_os = "windows")]
const BIN_PATH: &str = "bin/win64/";
#[cfg(target_os = "linux")]
const INDEX_PATH: &str = "bin/linux64/index.csv";
#[cfg(target_os = "linux")]
const BIN_PATH: &str = "bin/linux64";


fn main() {
	// default value, calculate new one
	let default_root = {
		let current_dir = std::env::current_dir().unwrap();
		if current_dir.ends_with(BIN_PATH) {
			// running directly from binaries directory, set root to ../..
			current_dir.parent().unwrap().parent().unwrap().to_str().unwrap().to_owned()
		} else if current_dir.ends_with("bin/") {
			current_dir.parent().unwrap().to_str().unwrap().to_owned()
		} else {
			current_dir.to_str().unwrap().to_owned()
		}
	};

	let matches = Command::new("install_checker")
		.author("ENDERZOMBI102 <enderzombi102.end@gmail.com>")
		.version("0.1.0")
		.about("A tool used to verify a game's install")
		.arg(Arg::new("new-index")
			.help("Creates a new index file")
			.long("new-index")
			.action(ArgAction::SetTrue)
			.default_value("false")
		)
		.arg(Arg::new("root")
			.help("The engine root directory")
			.long("root")
			.action(ArgAction::Set)
			.default_value(default_root)
		)
		.arg(Arg::new("excluded")
			.help("GLOB pattern(s) to exclude when creating the index")
			.long("exclude")
			.short('e')
			.action(ArgAction::Append)
			.requires("new-index")
		)
		.arg(Arg::new("index-loc")
			.help("The index file to use")
			.long("index")
			.short('i')
			.action(ArgAction::Set)
			.default_value(INDEX_PATH)
		)
		.arg(Arg::new("format")
			.help("Output format")
			.long("format")
			.short('f')
			.action(ArgAction::Set)
			.default_value("simple")
			.value_parser(PossibleValuesParser::new(["simple", "json", "csv"]))
		)
		.arg(Arg::new("overwrite")
			.help("Do not ask for confirmation for overwriting an existing index")
			.long("overwrite")
			.action(ArgAction::SetTrue)
			.requires("new-index")
		)
		.get_matches();

	let ignored = [
		String::from("sdk_content/**/*.*"),
		String::from("hammer/autosave/*.*"),
		String::from("**/index.csv"),
	];

	let index_location = matches.get_one::<String>("index-loc").unwrap();
	let root = matches.get_one::<String>("root").unwrap();

	let mut output: Box<dyn Output> = match matches.get_one::<String>("format").unwrap().as_str() {
		"simple" => SimpleOutput::new(),
		"json" => JsonOutput::new(),
		"csv" => CsvOutput::new(),
		it => {
			eprintln!("Invalid `--format` argument: `{it}`");
			std::process::exit(1);
		}
	};
	let ret: i32;

	output.init();
	if matches.get_flag("new-index") {
		let mut excludes = matches.get_many("excluded")
			.map(|it| it.copied().collect())
			.or(Some(Vec::new()))
			.unwrap();
		let overwite = matches.get_flag("overwrite");

		excludes.push(&ignored[0]);
		excludes.push(&ignored[1]);
		excludes.push(&ignored[2]);

		ret = create(root, index_location, excludes, overwite, &mut *output);
	} else {
		ret = verify(root, index_location, &mut *output);
	}
	output.end();

	std::process::exit(ret);
}
