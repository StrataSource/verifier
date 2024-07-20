//
// Created by ENDERZOMBI102 on 15/10/2023.
//
#include "verify.hpp"

#include <array>
#include <filesystem>
#include <fstream>

#include <cryptopp/crc.h>
#include <cryptopp/filters.h>
#include <cryptopp/hex.h>
#include <cryptopp/sha.h>
#include <sourcepp/crypto/String.h>
#include <vpkpp/format/VPK.h>

#include "log.hpp"

static auto verifyArchivedFile( const std::string& archivePath, const std::string& entryPath, std::uint64_t expectedSize, std::string_view expectedSha256, std::string_view expectedCrc32, unsigned int& entries, unsigned int& errors ) -> void;

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
	std::ifstream indexStream{ indexPath, std::ios::in };
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
		const auto& archive{ split[ 0 ] };
		const auto& pathRel{ split[ 1 ] };
		const auto expectedSize{ std::stoull( split[ 2 ] ) };
		const auto& expectedSha256{ split[ 3 ] };
		const auto& expectedCrc32{ split[ 4 ] };

		// verify it
		const bool insideArchive{ archive != "." };
		std::filesystem::path path{ insideArchive ? root / archive : root / pathRel };

		if (! std::filesystem::exists( path ) ) {
			Log_Report( pathRel, "Entry doesn't exist on disk.", "nul", "nul" );
			errors += 1;
			continue;
		}

		if ( insideArchive ) {
			verifyArchivedFile( path.string(), pathRel, expectedSize, expectedSha256, expectedCrc32, entries, errors );
			continue;
		}

		// open the file, but first close the old one if it is open
		if ( file )
			std::fclose( file );
#ifndef _WIN32
		file = std::fopen( path.string().c_str(), "rb" );
#else
		file = nullptr;
		fopen_s( &file, path.string().c_str(), "rb" );
#endif
		if (! file ) {
			Log_Error( "Failed to open file: `{}`", path.string() );
			continue;
		}

		std::fseek( file, 0, SEEK_END );

		auto length{ std::ftell( file ) };
		if ( length != expectedSize ) {
			Log_Report( pathRel, "Sizes don't match.", std::to_string( length ), std::to_string( expectedSize ) );
			Log_Info( "Processed entry `{}`", pathRel );
			entries += 1;
			errors += 1;
			continue;
		}
		std::fseek( file, 0, 0 );

		// sha256/crc32
		CryptoPP::SHA256 sha256er{};
		CryptoPP::CRC32 crc32er{};

		unsigned char buffer[2048];
		while ( unsigned int count = std::fread( buffer, 1, sizeof( buffer ), file ) ) {
			sha256er.Update( buffer, count );
			crc32er.Update( buffer, count );
		}

		std::array<CryptoPP::byte, CryptoPP::SHA256::DIGESTSIZE> sha256Hash{};
		sha256er.Final(sha256Hash.data());
		std::array<CryptoPP::byte, CryptoPP::CRC32::DIGESTSIZE> crc32Hash{};
		crc32er.Final(crc32Hash.data());

		std::string sha256HashStr;
		std::string crc32HashStr;
		{
			CryptoPP::StringSource sha256HashStrSink{ sha256Hash.data(), sha256Hash.size(), true, new CryptoPP::HexEncoder{ new CryptoPP::StringSink{ sha256HashStr } } };
			CryptoPP::StringSource crc32HashStrSink{ crc32Hash.data(), crc32Hash.size(), true, new CryptoPP::HexEncoder{ new CryptoPP::StringSink{ crc32HashStr } } };
		}

		if ( sha256HashStr != expectedSha256 ) {
			Log_Report( pathRel, "Content sha256 doesn't match.", sha256HashStr, expectedSha256 );
			errors += 1;
		}

		if ( crc32HashStr != expectedCrc32 ) {
			Log_Report( pathRel, "Content crc32 doesn't match.", crc32HashStr, expectedCrc32 );
			errors += 1;
		}

		Log_Info( "Processed file `{}`", pathRel );
		entries += 1;
	}
	std::fclose( file );

	auto end{ std::chrono::high_resolution_clock::now() };
	Log_Info( "Verified {} files in {} with {} errors!", entries, std::chrono::duration_cast<std::chrono::seconds>( end - start ), errors );

	return 0;
}

static auto verifyArchivedFile( const std::string& archivePath, const std::string& entryPath, std::uint64_t expectedSize, std::string_view expectedSha256, std::string_view expectedCrc32, unsigned int& entries, unsigned int& errors ) -> void {
	using namespace vpkpp;

	static std::unordered_map<std::string, std::unique_ptr<PackFile>> loadedVPKs{};
	if (! loadedVPKs.contains( archivePath ) ) {
		loadedVPKs[ archivePath ] = VPK::open( archivePath );
	}
	if (! loadedVPKs[ archivePath ]) {
		Log_Error( "Failed to open VPK at `{}` (containing file at `{}`)", archivePath, entryPath );
		return;
	}

	const auto fullPath{ archivePath + '/' + entryPath };

	auto entry = loadedVPKs[ archivePath ]->findEntry( entryPath );
	if (! entry ) {
		Log_Report( fullPath, "Entry doesn't exist on disk.", "nul", "nul" );
		errors += 1;
		return;
	}

	auto entryData = loadedVPKs[ archivePath ]->readEntry( *entry );
	if (! entryData ) {
		Log_Error( "Failed to open file: `{}`", fullPath );
		return;
	}

	if ( entryData->size() != expectedSize ) {
		Log_Report( fullPath, "Sizes don't match.", std::to_string( entryData->size() ), std::to_string( expectedSize ) );
		Log_Info( "Processed entry `{}`", fullPath );
		entries += 1;
		errors += 1;
		return;
	}

	// sha256 (crc32 is already computed)
	CryptoPP::SHA256 sha256er{};
	sha256er.Update( reinterpret_cast<const CryptoPP::byte*>( entryData->data() ), entryData->size() );
	std::array<CryptoPP::byte, CryptoPP::SHA256::DIGESTSIZE> sha256Hash{};
	sha256er.Final(sha256Hash.data());

	std::string sha256HashStr;
	std::string crc32HashStr;
	{
		CryptoPP::StringSource sha256HashStrSink{ sha256Hash.data(), sha256Hash.size(), true, new CryptoPP::HexEncoder{ new CryptoPP::StringSink{ sha256HashStr } } };
		CryptoPP::StringSource crc32HashStrSink{ reinterpret_cast<const CryptoPP::byte*>( &entry->crc32 ), sizeof( entry->crc32 ), true, new CryptoPP::HexEncoder{ new CryptoPP::StringSink{ crc32HashStr } } };
	}

	if ( sha256HashStr != expectedSha256 ) {
		Log_Report( fullPath, "Content sha256 doesn't match.", sha256HashStr, expectedSha256 );
		errors += 1;
	}

	if ( crc32HashStr != expectedCrc32 ) {
		Log_Report( fullPath, "Content crc32 doesn't match.", crc32HashStr, expectedCrc32 );
		errors += 1;
	}

	Log_Info( "Processed file `{}`", fullPath );
	entries += 1;
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
