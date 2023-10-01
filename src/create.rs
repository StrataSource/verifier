use std::io::Write;
use std::path::PathBuf;
use std::process::exit;

use sha2::Digest;

pub(crate) fn create(root: &String, index_location: &str, excluded: Vec<&String>) {
	let root = PathBuf::from(root);
	let index_path = root.join(index_location);

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
		let path = entry.unwrap();

		let path_str = path.to_str().unwrap();
		if !path.is_file() {
			continue;
		}

		// relative path
		let path_rel = path.strip_prefix(&root).unwrap().to_str().unwrap();

		for exclusion in &excluded {
			if glob_match::glob_match(exclusion, path_rel) {
				continue 'outer;
			}
		}

		// data-related columns
		let data = match std::fs::read(&path) {
			Ok(data) => data,
			Err(err) => {
				eprintln!("Error: Failed to read file `{path_str}`: {err}");
				continue;
			}
		};

		let size = data.len();

		let sha2 = {
			let mut sha256er = sha2::Sha256::new();
			sha256er.update(&data);
			sha256er.finalize()
		};

		let crc32 = {
			let mut crc32er = crc32fast::Hasher::new();
			crc32er.update(data.as_slice());
			crc32er.finalize()
		};

		writer.write(format!("{path_rel}, {size}, {sha2:x}, {crc32}\n").as_bytes()).unwrap();

		println!("Info: Processed file `{path_str}`");
		count += 1;
	}

	println!("Info: Finished processing {count} files in {}s!", start.elapsed().as_secs());
}
