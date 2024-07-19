//
// Created by ENDERZOMBI102 on 15/10/2023.
//
#include <filesystem>
#include <fstream>
#include <hash-library/crc32.h>
#include <hash-library/sha256.h>
#include <string_view>

#include "verify.hpp"
#include "log.hpp"


static auto splitString( const std::string& string, const std::string& delim ) -> std::vector<std::string>;

auto verify( std::string_view root_, std::string_view indexLocation ) -> int {
	const std::filesystem::path root{ root_ };
	const std::filesystem::path indexPath{ root / indexLocation };

	if (! std::filesystem::exists( indexPath ) ) {
		Log_Error( "Index file `{}` does not exist.", indexPath.string() );
		return 1;
	}

	Log_Info( "Using index file at `{}`", indexPath.string() );

	// open index file, if the file didn't exist, we wouldn't be here
	std::ifstream indexStream{ indexPath, std::ios_base::in };
	if (! indexStream.good() ) {
		Log_Error( "Failed to open index file for reading: N/D" );
		return 1;
	}

	// working variables for the checking step
	unsigned entries{ 0 };
	unsigned errors{ 0 };
	auto start{ std::chrono::high_resolution_clock::now() };

	// read and verify
	std::stringbuf line{};
	std::FILE* file{ nullptr };
	while (! indexStream.eof() ) {
		line.pubseekpos( 0 );
		// read row data
		indexStream.get( line, '\xFD' );
		indexStream.get(); // value end separator
		if ( line.in_avail() == 0 )
			break;

		const auto split{ splitString( line.str(), "\xFF" ) };
		// deserialize row
		const auto& pathRel{ split[ 0 ] };
		const auto expectedSize{ strtoull( split[ 1 ].c_str(), nullptr, 10 ) };
		const auto& expectedSha256{ split[ 2 ] };
		const auto& expectedCrc32{ split[ 3 ] };

		// verify it
		const auto path = root / pathRel;

		if (! std::filesystem::exists( path ) ) {
			Log_Report( pathRel, "Entry doesn't exist on disk.", "nul", "nul" );
			errors += 1;
			continue;
		}

		// open the file, but first close the old one if it is open
		if ( file )
			std::fclose( file );
#ifndef _WIN32
		std::FILE* file{ std::fopen( path.string().c_str(), "rb" ) };
#else
		file = nullptr;
		fopen_s( &file, path.string().c_str(), "rb" );
#endif
		if (! file ) {
			Log_Error( "Failed to open file: {}", path.string() );
			continue;
		}

		std::fseek( file, 0, SEEK_END );

		auto length{ std::ftell( file ) };
		if ( length != expectedSize ) {
			Log_Report( pathRel, "Sizes don't match.", std::to_string( length ), split[ 3 ] );
			Log_Info( "Processed entry `{}`", pathRel );
			entries += 1;
			errors += 1;
			continue;
		}
		std::fseek( file, 0, 0 );

		SHA256 sha256er{};
		CRC32 crc32er{};
		unsigned char buffer[16] { 0 };
		unsigned count;
		while ( (count = std::fread( buffer, 1, 16, file )) != 0 ) {
			sha256er.add( buffer, count );
			crc32er.add( buffer, count );
		}

		if ( sha256er.getHash() != expectedSha256 ) {
			Log_Report( pathRel, "Content sha256 doesn't match.", sha256er.getHash(), expectedSha256 );
			errors += 1;
		}

		if ( crc32er.getHash() != expectedCrc32 ) {
			Log_Report( pathRel, "Content crc32 doesn't match.", sha256er.getHash(), expectedSha256 );
			errors += 1;
		}

		Log_Info( "Processed entry `{}`", pathRel );
		entries += 1;
	}
	std::fclose( file );

	auto end{ std::chrono::high_resolution_clock::now() };
	Log_Info( "Verified {} files in {} with {} errors!", entries, std::chrono::duration_cast<std::chrono::seconds>( end - start ), errors );

	return 0;
}

static auto splitString( const std::string& string, const std::string& delim ) -> std::vector<std::string> {
	std::vector<std::string> res{};
	size_t last{ 0 };
	size_t end;

	while ( ( end = string.find( delim, last ) ) != std::string::npos ) {
		// ptr + lastOffset -> segment start, offset - lastOffset -> size
		res.push_back( string.substr( last, end - last ) );
		last = end + delim.length();
	}

	res.push_back( string.substr( last, string.length() - last ) );
	return res;
}