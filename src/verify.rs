use std::path::PathBuf;
use std::process::exit;
use std::rc::Rc;

use sha2::Digest;
use serde_derive::{Deserialize, Serialize};

#[derive(Debug, Deserialize, Serialize)]
struct IndexEntry {
	path: Rc<str>,
	size: usize,
	sha2: Box<str>,
	crc32: u32,
}

struct Report(Rc<str>, &'static str);

pub(crate) fn verify(root: &String) {
	let root = PathBuf::from(root);
	// correct the index path with os
	#[cfg(target_os = "windows")]
		let index_path = root.join("bin/win64/index.csv");
	#[cfg(target_os = "linux")]
		let index_path = root.join("bin/linux64/index.csv");

	if !index_path.exists() {
		eprintln!("Error: Index file `{}` does not exist.", index_path.to_str().unwrap());
		exit(1);
	}

	println!( "Info: Using index file at `{}`", index_path.to_str().unwrap() );

	// open index file with a csv reader
	let mut reader = match csv::Reader::from_path(index_path) {
		Ok(rdr) => rdr,
		Err(err) => {
			eprintln!("Error: Failed to open index file for reading: {err}");
			exit(1);
		}
	};

	// working variables for the checking step
	let header = csv::StringRecord::from(vec!["path", "size", "sha2", "crc32"]);
	let mut reports: Vec<Report> = Vec::new();
	let mut count = 0u32;
	let start = std::time::Instant::now();

	// read and verify
	for entry in reader.records() {
		// read entry from csv file
		let record = match entry {
			Ok(it) => it,
			Err(err) => {
				eprintln!("Error: Failed to read index entry: {err}");
				continue;
			}
		};

		// deserialize entry
		let entry = match record.deserialize::<IndexEntry>(Some(&header)) {
			Ok(entry) => entry,
			Err(err) => {
				eprintln!("Error: Failed to deserialize row: {err}");
				continue;
			}
		};

		// verify it
		let path = root.join(&*entry.path);

		if !path.exists() {
			reports.push(Report(entry.path, "Entry doesn't exist on disk."));
			continue;
		}

		let Ok(data) = std::fs::read(path) else {
			reports.push(Report(entry.path, "Failed to read file."));
			continue;
		};

		if data.len() != entry.size {
			reports.push(Report(entry.path, "Sizes don't match."));
			continue;
		}

		let mut sha256er = sha2::Sha256::new();
		sha256er.update(&data);
		if format!("{:x}", sha256er.finalize()).as_str() != &*entry.sha2 {
			reports.push(Report(entry.path.clone(), "Content crc32 doesn't match."));
		}

		let mut crc32er = crc32fast::Hasher::new();
		crc32er.update(data.as_slice());
		if crc32er.finalize() != entry.crc32 {
			reports.push(Report(entry.path.clone(), "Content crc32 doesn't match."));
		}

		println!("Info: Processed entry `{}`", entry.path);
		count += 1;
	}

	for report in reports {
		eprintln!( "In file `{}`: {}", report.0, report.1 );
	}

	println!("Info: Verified {count} files in {}s!", start.elapsed().as_secs());
}
