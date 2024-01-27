//
// Created by ENDERZOMBI102 on 15/10/2023.
//
#include <crc32.h>
#include <filesystem>
#include <fmt/chrono.h>
#include <fmt/format.h>
#include <fstream>
#include <iostream>
#include <ranges>
#include <sha256.h>
#include <string_view>
#include <regex>

#include "create.hpp"


auto create( std::string_view root_, std::string_view indexLocation, const std::vector<std::string>& excluded, bool overwrite, const Output* out ) noexcept -> int {
	const std::filesystem::path root{ root_ };
	const std::filesystem::path indexPath{ root / indexLocation };

	if ( std::filesystem::exists( indexPath ) ) {
		if (! overwrite ) {
			out->write( OutputKind::Error, fmt::format( "Index file `{}` already exist, do you want to overwite it? (y/N)", indexPath.string() ) );
			std::string input;
			std::cin >> input;
			if ( input != "y" ) {
				out->write( OutputKind::Info, "Aborting." );
				return 1;
			}
		} else {
			out->write( OutputKind::Warn, fmt::format( "Index file `{}` already exist, will be overwritten.", indexPath.string() ) );
		}
	}

	auto start{ std::chrono::high_resolution_clock::now() };
	out->write( OutputKind::Info, fmt::format( "Creating index file at `{}`", indexPath.string() ) );

	// open index file with a writer stream
	std::ofstream writer{ indexPath, std::ios_base::out | std::ios_base::trunc };
	if (! writer.good() ) {
		out->write( OutputKind::Error, fmt::format( "Failed to open index file for writing: N/D" ) );
		return 1;
	}

	writer << "path, size, sha2, crc32\n";
	out->write( OutputKind::Info, "Compiling exclusion regexes..." );
	// allocate all at once
	std::vector<std::regex> exclusionREs{};
	exclusionREs.reserve( excluded.size() );
	for ( const auto& exclusion : excluded ) {
		exclusionREs.emplace_back( exclusion, std::regex::ECMAScript | std::regex::icase | std::regex::optimize );
	}
	out->write( OutputKind::Info, fmt::format( "Done in ", std::chrono::duration_cast<std::chrono::seconds>( std::chrono::high_resolution_clock::now() - start ) ) );

	unsigned count{ 0 };
	unsigned errors{ 0 };
	// read and create index
	std::filesystem::recursive_directory_iterator iterator{ root };
	for ( const auto& entry : iterator ) {
		const auto& path{ entry.path() };

		if (! std::filesystem::is_regular_file( path ) ) {
			continue;
		}

		// relative path
		const auto pathRel = std::filesystem::relative(path, root).string();

		auto breaker{ false };
		for ( const auto& exclusion : exclusionREs ) {
			if ( std::regex_match( pathRel, exclusion ) ) {
				breaker = true;
				break;
			}
		}
		if ( breaker )
			continue;

		// data-related columns
		std::FILE* file{ fopen( path.c_str(), "rb" ) };
		std::fseek( file, 0, SEEK_END );

		const auto size{ std::ftell( file ) };

		std::fseek( file, 0, 0 );

		SHA256 sha256er{};
		CRC32 crc32er{};
		unsigned char buffer[16] { 0 };
		unsigned bufCount;
		while ( (bufCount = std::fread( buffer, 1, 16, file )) != 0 ) {
			sha256er.add( buffer, bufCount );
			crc32er.add( buffer, bufCount );
		}

		const auto sha256{ sha256er.getHash() };
		const auto crc32{ crc32er.getHash() };

		writer << fmt::format( "{}\xFF{}\xFF{}\xFF{}\xFF\xFD", pathRel, size, sha256, crc32 );
		out->write( OutputKind::Info, fmt::format( "Processed file `{}`", path.string() ) );
		count += 1;
	}

	auto end{ std::chrono::high_resolution_clock::now() };
	out->write(
		OutputKind::Info,
		fmt::format(
			"Finished processing {} files in {}! (with {} errors)",
			count,
			std::chrono::duration_cast<std::chrono::seconds>(end - start),
			errors
		)
	);

	return 0;
}
