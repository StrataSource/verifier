//
// Created by ENDERZOMBI102 on 15/10/2023.
//
#include "create.hpp"

#include <array>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <string_view>

#include <cryptopp/crc.h>
#include <cryptopp/filters.h>
#include <cryptopp/hex.h>
#include <cryptopp/sha.h>
#include <sourcepp/parser/Text.h>
#include <sourcepp/string/String.h>
#include <vpkpp/format/VPK.h>

#include "log.hpp"

static auto enterVPK( std::ofstream& writer, std::string_view vpkPath, std::string_view vpkPathRel, const std::vector<std::regex>& excluded, const std::vector<std::regex>& included ) -> bool;

auto createFromRoot( std::string_view root_, std::string_view indexLocation, bool skipArchives, const std::vector<std::string>& excluded, const std::vector<std::string>& included ) -> int {
	const std::filesystem::path root{ root_ };
	const std::filesystem::path indexPath{ root / indexLocation };

	auto start{ std::chrono::high_resolution_clock::now() };
	Log_Info( "Creating index file at `{}`", indexPath.string() );

	// open index file with a writer stream
	std::ofstream writer{ indexPath, std::ios::out | std::ios::app };
	if (! writer.good() ) {
		Log_Error( "Failed to open index file for writing: N/D" );
		return 1;
	}

	Log_Info( "Compiling exclusion regexes..." );
	std::vector<std::regex> exclusionREs;
	exclusionREs.reserve( excluded.size() );
	for ( const auto& exclusion : excluded ) {
		exclusionREs.emplace_back( exclusion, std::regex::ECMAScript | std::regex::icase | std::regex::optimize );
	}
	if (! skipArchives ) {
		exclusionREs.emplace_back( R"(.*_[0-9][0-9][0-9]\.vpk)", std::regex::ECMAScript | std::regex::icase | std::regex::optimize );
	}

	Log_Info( "Compiling inclusion regexes..." );
	std::vector<std::regex> inclusionREs;
	if (! included.empty() ) {
		inclusionREs.reserve( included.size() );
		for ( const auto& inclusion: included ) {
			inclusionREs.emplace_back( inclusion, std::regex::ECMAScript | std::regex::icase | std::regex::optimize );
		}
	}

	Log_Info( "Done in {}", std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::high_resolution_clock::now() - start ) );

	unsigned count{ 0 };
	unsigned errors{ 0 };
	// read and create index
	std::filesystem::recursive_directory_iterator iterator{ root };
	for ( const auto& entry : iterator ) {
		auto path{ entry.path().string() };
		sourcepp::string::normalizeSlashes( path );

		if (! std::filesystem::is_regular_file( path ) ) {
			continue;
		}

		// relative path
		auto pathRel{ std::filesystem::relative( path, root ).string() };
		sourcepp::string::normalizeSlashes( pathRel );

		auto breaker{ false };
		for ( const auto& exclusion : exclusionREs ) {
			if ( std::regex_match( pathRel, exclusion ) ) {
				breaker = true;
				break;
			}
		}
		if ( breaker )
			continue;

		if (! inclusionREs.empty() ) {
			for ( const auto& inclusion: inclusionREs ) {
				if ( std::regex_match( pathRel, inclusion ) ) {
					breaker = true;
					break;
				}
			}
			if (! breaker )
				continue;
		}

		if ( !skipArchives && path.ends_with( ".vpk" ) ) {
			// We've already ignored numbered archives in the exclusion regexes
			if ( enterVPK( writer, path, pathRel, exclusionREs, inclusionREs ) ) {
				Log_Info( "Processed VPK at `{}`", path );
				continue;
			}
			Log_Warn( "Unable to open VPK at `{}`. Treating as a regular file...", path );
		}

		// open file
#ifndef _WIN32
		std::FILE* file{ std::fopen( path.c_str(), "rb" ) };
#else
		std::FILE* file{ nullptr };
		fopen_s( &file, path.c_str(), "rb" );
#endif
		if (! file ) {
			Log_Error( "Failed to open file: `{}`", path );
			continue;
		}

		// data-related columns
		// size
		std::fseek( file, 0, SEEK_END );
		const auto size{ std::ftell( file ) };
		std::fseek( file, 0, 0 );

		// sha256/crc32
		CryptoPP::SHA256 sha256er{};
		CryptoPP::CRC32 crc32er{};

		unsigned char buffer[2048];
		while ( auto bufCount = std::fread( buffer, 1, sizeof( buffer ), file ) ) {
			sha256er.Update( buffer, bufCount );
			crc32er.Update( buffer, bufCount );
		}
		std::fclose( file );

		std::array<CryptoPP::byte, CryptoPP::SHA256::DIGESTSIZE> sha256Hash{};
		sha256er.Final( sha256Hash.data() );
		std::array<CryptoPP::byte, CryptoPP::CRC32::DIGESTSIZE> crc32Hash{};
		crc32er.Final( crc32Hash.data() );

		std::string sha256HashStr;
		std::string crc32HashStr;
		{
			CryptoPP::StringSource sha256HashStrSink{ sha256Hash.data(), sha256Hash.size(), true, new CryptoPP::HexEncoder{ new CryptoPP::StringSink{ sha256HashStr } } };
			CryptoPP::StringSource crc32HashStrSink{ crc32Hash.data(), crc32Hash.size(), true, new CryptoPP::HexEncoder{ new CryptoPP::StringSink{ crc32HashStr } } };
		}

		// write out entry
		writer << fmt::format( ".\xFF{}\xFF{}\xFF{}\xFF{}\xFF\xFD", pathRel, size, sha256HashStr, crc32HashStr );
		Log_Info( "Processed file `{}`", path );
		count += 1;
	}

	auto end{ std::chrono::high_resolution_clock::now() };
	Log_Info( "Finished processing {} files in {}! (with {} errors)", count, std::chrono::duration_cast<std::chrono::seconds>( end - start ), errors );

	return 0;
}

static auto enterVPK( std::ofstream& writer, std::string_view vpkPath, std::string_view vpkPathRel, const std::vector<std::regex>& excluded, const std::vector<std::regex>& included ) -> bool {
	using namespace vpkpp;

	auto vpk = VPK::open( std::string{ vpkPath } );
	if (! vpk ) {
		return false;
	}

	for ( const auto& [ entryDirectory, entries ] : vpk->getBakedEntries() ) {
		for ( const auto& entry : entries ) {
			auto breaker{ false };
			for ( const auto& exclusion : excluded ) {
				if ( std::regex_match( entry.path, exclusion ) ) {
					breaker = true;
					break;
				}
			}
			if ( breaker )
				continue;

			if (! included.empty() ) {
				for ( const auto& inclusion: included ) {
					if ( std::regex_match( entry.path, inclusion ) ) {
						breaker = true;
						break;
					}
				}
				if (! breaker )
					continue;
			}

			auto entryData{ vpk->readEntry( entry ) };
			if (! entryData ) {
				Log_Error( "Failed to open file: `{}/{}`", vpkPath, entry.path );
				continue;
			}

			// sha256 (crc32 is already computed)
			CryptoPP::SHA256 sha256er{};
			sha256er.Update( reinterpret_cast<const CryptoPP::byte*>( entryData->data() ), entryData->size() );
			std::array<CryptoPP::byte, CryptoPP::SHA256::DIGESTSIZE> sha256Hash{};
			sha256er.Final( sha256Hash.data() );

			std::string sha256HashStr;
			std::string crc32HashStr;
			{
				CryptoPP::StringSource sha256HashStrSink{ sha256Hash.data(), sha256Hash.size(), true, new CryptoPP::HexEncoder{ new CryptoPP::StringSink{ sha256HashStr } } };
				CryptoPP::StringSource crc32HashStrSink{ reinterpret_cast<const CryptoPP::byte*>( &entry.crc32 ), sizeof( entry.crc32 ), true, new CryptoPP::HexEncoder{ new CryptoPP::StringSink{ crc32HashStr } } };
			}

			// write out entry
			writer << fmt::format( "{}\xFF{}\xFF{}\xFF{}\xFF{}\xFF\xFD", vpkPathRel, entry.path, entryData->size(), sha256HashStr, crc32HashStr );
			Log_Info( "Processed file `{}/{}`", vpkPath, entry.path );
		}
	}

	return true;
}
