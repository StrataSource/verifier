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

static auto enterVPK( std::ofstream& writer, std::string_view vpkPath, std::string_view vpkPathRel, const std::vector<std::regex>& excludes, const std::vector<std::regex>& includes ) -> bool;
static auto buildRegexCollection( const std::vector<std::string>& regexStrings, std::string_view collectionType ) -> std::vector<std::regex>;
static auto matchPath( const std::string& path, const std::vector<std::regex>& regexes ) -> bool;

auto createFromRoot( std::string_view root_, std::string_view indexLocation, bool skipArchives,
					 const std::vector<std::string>& fileExcludes, const std::vector<std::string>& fileIncludes,
					 const std::vector<std::string>& archiveExcludes, const std::vector<std::string>& archiveIncludes ) -> int {
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

	std::vector<std::regex> archiveExclusionREs = buildRegexCollection(archiveExcludes, "archive exclusion");
	std::vector<std::regex> archiveInclusionREs = buildRegexCollection(archiveIncludes, "archive inclusion");
	std::vector<std::regex> fileExclusionREs = buildRegexCollection(fileExcludes, "file exclusion");
	std::vector<std::regex> fileInclusionREs = buildRegexCollection(fileIncludes, "file inclusion");

	// We always pass some regexes in from main.cpp, so not need for an ugly check if we actually
	// compiled anything - fileExclusionREs will always be non-empty.
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

		if ( path.ends_with( ".vpk" ) ) {
			static const std::regex numberedVpkRegex { R"(.*_[0-9][0-9][0-9]\.vpk)", std::regex::ECMAScript | std::regex::icase | std::regex::optimize };

			if ( skipArchives || std::regex_match( pathRel, numberedVpkRegex ) ) {
				continue;
			}

			if ( !archiveExclusionREs.empty() && matchPath( pathRel, archiveExclusionREs ) ) {
				continue;
			}

			if ( !archiveExclusionREs.empty() && !matchPath( pathRel, archiveInclusionREs ) ) {
				continue;
			}

			if ( enterVPK( writer, path, pathRel, fileExclusionREs, fileInclusionREs ) ) {
				Log_Info( "Processed VPK at `{}`", path );
				continue;
			}

			Log_Warn( "Unable to open VPK at `{}`. Treating as a regular file...", path );
		} else {
			if ( !fileInclusionREs.empty() && matchPath( pathRel, fileExclusionREs ) ) {
				continue;
			}

			if ( !fileExclusionREs.empty() && !matchPath( pathRel, fileInclusionREs ) ) {
				continue;
			}
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

		// sha1/crc32
		CryptoPP::SHA1 sha1er{};
		CryptoPP::CRC32 crc32er{};

		unsigned char buffer[2048];
		while ( auto bufCount = std::fread( buffer, 1, sizeof( buffer ), file ) ) {
			sha1er.Update( buffer, bufCount );
			crc32er.Update( buffer, bufCount );
		}
		std::fclose( file );

		std::array<CryptoPP::byte, CryptoPP::SHA1::DIGESTSIZE> sha1Hash{};
		sha1er.Final( sha1Hash.data() );
		std::array<CryptoPP::byte, CryptoPP::CRC32::DIGESTSIZE> crc32Hash{};
		crc32er.Final( crc32Hash.data() );

		std::string sha1HashStr;
		std::string crc32HashStr;
		{
			CryptoPP::StringSource sha1HashStrSink{ sha1Hash.data(), sha1Hash.size(), true, new CryptoPP::HexEncoder{ new CryptoPP::StringSink{ sha1HashStr } } };
			CryptoPP::StringSource crc32HashStrSink{ crc32Hash.data(), crc32Hash.size(), true, new CryptoPP::HexEncoder{ new CryptoPP::StringSink{ crc32HashStr } } };
		}

		// write out entry
		writer << fmt::format( ".\xFF{}\xFF{}\xFF{}\xFF{}\xFF\xFD", pathRel, size, sha1HashStr, crc32HashStr );
		Log_Info( "Processed file `{}`", path );
		count += 1;
	}

	auto end{ std::chrono::high_resolution_clock::now() };
	Log_Info( "Finished processing {} files in {}! (with {} errors)", count, std::chrono::duration_cast<std::chrono::seconds>( end - start ), errors );

	return 0;
}

static auto enterVPK( std::ofstream& writer, std::string_view vpkPath, std::string_view vpkPathRel, const std::vector<std::regex>& excludes, const std::vector<std::regex>& includes ) -> bool {
	using namespace vpkpp;

	auto vpk = VPK::open( std::string{ vpkPath } );
	if (! vpk ) {
		return false;
	}

	for ( const auto& [ entryDirectory, entries ] : vpk->getBakedEntries() ) {
		for ( const auto& entry : entries ) {
			if ( !excludes.empty() && matchPath( entry.path, excludes) ) {
				continue;
			}

			if ( !includes.empty() && !matchPath( entry.path, includes ) ) {
				continue;
			}

			auto entryData{ vpk->readEntry( entry ) };
			if (! entryData ) {
				Log_Error( "Failed to open file: `{}/{}`", vpkPath, entry.path );
				continue;
			}

			// sha1 (crc32 is already computed)
			CryptoPP::SHA1 sha1er{};
			sha1er.Update( reinterpret_cast<const CryptoPP::byte*>( entryData->data() ), entryData->size() );
			std::array<CryptoPP::byte, CryptoPP::SHA1::DIGESTSIZE> sha1Hash{};
			sha1er.Final( sha1Hash.data() );

			std::string sha1HashStr;
			std::string crc32HashStr;
			{
				CryptoPP::StringSource sha1HashStrSink{ sha1Hash.data(), sha1Hash.size(), true, new CryptoPP::HexEncoder{ new CryptoPP::StringSink{ sha1HashStr } } };
				CryptoPP::StringSource crc32HashStrSink{ reinterpret_cast<const CryptoPP::byte*>( &entry.crc32 ), sizeof( entry.crc32 ), true, new CryptoPP::HexEncoder{ new CryptoPP::StringSink{ crc32HashStr } } };
			}

			// write out entry
			writer << fmt::format( "{}\xFF{}\xFF{}\xFF{}\xFF{}\xFF\xFD", vpkPathRel, entry.path, entryData->size(), sha1HashStr, crc32HashStr );
			Log_Info( "Processed file `{}/{}`", vpkPath, entry.path );
		}
	}

	return true;
}

static auto buildRegexCollection( const std::vector<std::string>& regexStrings, std::string_view collectionType ) -> std::vector<std::regex> {
	std::vector<std::regex> collection {};

	if ( !regexStrings.empty() ) {
		Log_Info( "Compiling {} regexes...", collectionType );
		collection.reserve( regexStrings.size() );

		for ( const auto& item : regexStrings ) {
			collection.emplace_back( item, std::regex::ECMAScript | std::regex::icase | std::regex::optimize );
		}
	}

	return collection;
}

static auto matchPath( const std::string& path, const std::vector<std::regex>& regexes ) -> bool {
	return std::any_of( regexes.begin(), regexes.end(), [&]( const auto& item ) {
		return std::regex_match( path, item );
	});
}
