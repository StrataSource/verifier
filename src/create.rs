use std::io::Write;
use std::path::PathBuf;
use std::process::exit;

use sha2::Digest;

pub(crate) fn create(root: &String, excluded: Vec<&String>) {
	let root = PathBuf::from(root);
	// correct the index path with os
	#[cfg(target_os = "windows")]
		let index_path = root.join("bin/win64/index.csv");
	#[cfg(target_os = "linux")]
		let index_path = root.join("bin/linux64/index.csv");

	if index_path.exists() {
		eprintln!("Warning: Index file `{}` already exist, will be overwritten.", index_path.to_str().unwrap());
	}

	println!("Info: Creating index file at `{}`", index_path.to_str().unwrap());

	// open index file with a writer stream
	let mut writer = match std::fs::File::create(index_path) {
		Ok(writer) => writer,
		Err(err) => {
			eprintln!("Error: Failed to open index file for writing: {err}");
			exit(1);
		}
	};

	writer.write( "path, size, sha2, crc32\n".as_bytes() ).unwrap();
	let start = std::time::Instant::now();

	let mut count = 0u32;
	// read and verify
	'outer: for entry in glob::glob(&format!("{}/**/*.*", root.to_str().unwrap())).unwrap() {
		let path = entry.expect("aaaaa");
		let path_str = path.to_str().expect("eeeee");

		if !path.is_file() {
			println!("sddd {path:?}");
			continue;
		}

		for exclusion in &excluded {
			if glob_match::glob_match(exclusion, path_str) {
				continue 'outer;
			}
		}

		// verify it
		let data = match std::fs::read(&path) {
			Ok(data) => data,
			Err(err) => {
				eprintln!("Error: Failed to read file `{path_str}`: {err}");
				continue;
			}
		};

		let rel_path = path.strip_prefix(&root).unwrap().to_str().unwrap();

		let size = data.len();

		let mut sha256er = sha2::Sha256::new();
		sha256er.update(&data);
		let sha2 = sha256er.finalize();

		let mut crc32er = crc32fast::Hasher::new();
		crc32er.update(data.as_slice());
		let crc32 = crc32er.finalize();

		writer.write(format!("{rel_path}, {size}, {sha2:x}, {crc32}\n").as_bytes()).unwrap();

		println!("Info: Processed entry `{path_str}`");
		count += 1;
	}

	println!("Info: Finished processing {count} files in {}s!", start.elapsed().as_secs());
}
