use std::path::PathBuf;

use sha2::Digest;

use crate::output::{Output, OutputKind};

pub(crate) fn verify(root: &String, index_location: &str, out: &mut dyn Output) -> i32 {
	let root = PathBuf::from(root);
	let index_path = root.join(index_location);

	if !index_path.exists() {
		out.write(
			OutputKind::Error,
			format!("Index file `{}` does not exist.", index_path.to_str().unwrap()),
		);
		return 1;
	}

	out.write(
		OutputKind::Info,
		format!("Using index file at `{}`", index_path.to_str().unwrap()),
	);

	// open index file
	let index_data = match std::fs::read_to_string(index_path) {
		Ok(rdr) => rdr,
		Err(err) => {
			out.write(
				OutputKind::Error,
				format!("Failed to open index file for reading: {err}"),
			);
			return 1;
		}
	};

	// working variables for the checking step
	let mut entries = 0u32;
	let mut errors = 0u32;
	let start = std::time::Instant::now();

	// read and verify
	for row in index_data.lines().skip(1) {
		// deserialize row
		let split: Vec<&str> = row.split(", ").collect();
		let path_rel = split[0];
		let expected_size = split[1].parse().unwrap();
		let expected_sha256 = split[2];
		let expected_crc32 = split[3].parse().unwrap();

		// verify it
		let path = root.join(path_rel.clone());

		if !path.exists() {
			out.report(path_rel.clone(), "Entry doesn't exist on disk.", "null", "null");
			errors += 1;
			continue;
		}

		let Ok(data) = std::fs::read(path) else {
			out.report(path_rel.clone(), "Failed to read file.", "null", "null");
			errors += 1;
			continue;
		};

		if data.len() != expected_size {
			out.report(
				path_rel.clone(),
				"Sizes don't match.",
				data.len().to_string().as_str(),
				expected_size.to_string().as_str(),
			);
			errors += 1;
			continue;
		}

		let sha256 = {
			let mut sha256er = sha2::Sha256::new();
			sha256er.update(&data);
			format!("{:x}", sha256er.finalize())
		};
		if sha256.as_str() != expected_sha256 {
			out.report(
				path_rel.clone(),
				"Content crc32 doesn't match.",
				sha256.as_str(),
				expected_sha256
			);
			errors += 1;
		}

		let crc32 = {
			let mut crc32er = crc32fast::Hasher::new();
			crc32er.update(data.as_slice());
			crc32er.finalize()
		};
		if crc32 != expected_crc32 {
			out.report(
				path_rel.clone(),
				"Content crc32 doesn't match.",
				crc32.to_string().as_str(),
				expected_crc32.to_string().as_str()
			);
			errors += 1;
		}

		out.write(OutputKind::Info, format!("Processed entry `{}`", path_rel));
		entries += 1;
	}

	out.write(
		OutputKind::Info,
		format!("Verified {entries} files in {}s with {errors} errors!", start.elapsed().as_secs()),
	);
	0
}
