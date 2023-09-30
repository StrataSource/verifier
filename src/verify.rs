use std::path::PathBuf;
use std::process::exit;

use sha2::Digest;

struct Report<'a>(&'a str, &'static str);

pub(crate) fn verify(root: &String, index_location: &str) {
	let root = PathBuf::from(root);
	let index_path = root.join(index_location);

	if !index_path.exists() {
		eprintln!("Error: Index file `{}` does not exist.", index_path.to_str().unwrap());
		exit(1);
	}

	println!( "Info: Using index file at `{}`", index_path.to_str().unwrap() );

	// open index file
	let index_data = match std::fs::read_to_string(index_path) {
		Ok(rdr) => rdr,
		Err(err) => {
			eprintln!("Error: Failed to open index file for reading: {err}");
			exit(1);
		}
	};

	// working variables for the checking step
	let mut reports: Vec<Report> = Vec::new();
	let mut count = 0u32;
	let start = std::time::Instant::now();

	// read and verify
	for row in index_data.lines().skip(1) {
		// deserialize row
		let split: Vec<&str> = row.split(", ").collect();
		let path_rel = split[0];
		let size = split[1].parse().unwrap();
		let sha2 = split[2];
		let crc32 = split[3].parse().unwrap();

		// verify it
		let path = root.join(path_rel.clone());

		if !path.exists() {
			reports.push(Report(path_rel.clone(), "Entry doesn't exist on disk."));
			continue;
		}

		let Ok(data) = std::fs::read(path) else {
			reports.push(Report(path_rel.clone(), "Failed to read file."));
			continue;
		};

		if data.len() != size {
			reports.push(Report(path_rel.clone(), "Sizes don't match."));
			continue;
		}

		let mut sha256er = sha2::Sha256::new();
		sha256er.update(&data);
		if format!("{:x}", sha256er.finalize()).as_str() != &*sha2 {
			reports.push(Report(path_rel.clone(), "Content crc32 doesn't match."));
		}

		let mut crc32er = crc32fast::Hasher::new();
		crc32er.update(data.as_slice());
		if crc32er.finalize() != crc32 {
			reports.push(Report(path_rel.clone(), "Content crc32 doesn't match."));
		}

		println!("Info: Processed entry `{}`", path_rel);
		count += 1;
	}

	for report in &reports {
		eprintln!( "In file `{}`: {}", report.0, report.1 );
	}

	println!("Info: Verified {count} files in {}s with {} errors!", start.elapsed().as_secs(), reports.len());
}
