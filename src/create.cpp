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
#include <kvpp/kvpp.h>
#include <sourcepp/FS.h>
#include <vpkpp/format/VPK.h>

#include "log.hpp"

static auto enterVPK( std::ofstream& writer, std::string_view vpkPath, std::string_view vpkPathRel, const std::vector<std::regex>& excludes, const std::vector<std::regex>& includes, unsigned int& count ) -> bool;
static auto buildRegexCollection( const std::vector<std::string>& regexStrings, std::string_view collectionType ) -> std::vector<std::regex>;
static auto matchPath( const std::string& path, const std::vector<std::regex>& regexes ) -> bool;
static auto globToRegex( std::string_view glob ) -> std::string;

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

	std::vector<std::regex> archiveExclusionREs = buildRegexCollection( archiveExcludes, "archive exclusion" );
	std::vector<std::regex> archiveInclusionREs = buildRegexCollection( archiveIncludes, "archive inclusion" );
	std::vector<std::regex> fileExclusionREs = buildRegexCollection( fileExcludes, "file exclusion" );
	std::vector<std::regex> fileInclusionREs = buildRegexCollection( fileIncludes, "file inclusion" );

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

		if ( ( !fileExclusionREs.empty() && matchPath( pathRel, fileExclusionREs ) ) || ( !fileInclusionREs.empty() && !matchPath( pathRel, fileInclusionREs ) ) ) {
			// File is either excluded or not included
			continue;
		}

		if ( !skipArchives && path.ends_with( ".vpk" ) ) {
			if ( enterVPK( writer, path, pathRel, archiveExclusionREs, archiveInclusionREs, count ) ) {
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

		// sha1/crc32
		CryptoPP::SHA1 sha1er{};
		CryptoPP::CRC32 crc32er{};

		unsigned char buffer[ 2048 ];
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
		Log_Verbose( "Processed file `{}`", path );
		count += 1;
	}

	auto end{ std::chrono::high_resolution_clock::now() };
	Log_Info( "Finished processing {} files in {}! (with {} errors)", count, std::chrono::duration_cast<std::chrono::seconds>( end - start ), errors );

	return 0;
}

auto createFromSteamDepotConfigs( const std::string& configPath, const std::vector<std::string>& depotIDs, std::string_view indexLocation,
								  bool skipArchives, const std::vector<std::string>& fileExcludes, const std::vector<std::string>& fileIncludes,
								  const std::vector<std::string>& archiveExcludes, const std::vector<std::string>& archiveIncludes ) -> int {
	using namespace kvpp;

	/*
	 * We don't support the full depot config spec (most of it though!)
	 *
	 * Assumptions are as follows:
	 *  - ContentRoot (wherever it appears) is relative
	 *  - DepotBuildConfig/FileMapping/LocalPath is relative
	 *  - DepotBuildConfig/FileMapping/LocalPath contains files directly under DepotBuildConfig/FileMapping/DepotPath
	 *    (for example, LocalPath is ".\p2ce\x.txt" and DepotPath is ".\p2ce\")
	 *  - DepotBuildConfig/FileMapping/Recursive is true when LocalPath is a directory
	 */
	Log_Warn( "The Steam depot config parser is incomplete and fine-tuned to work with Portal 2: Community Edition. Use at your own risk." );

	auto start{ std::chrono::high_resolution_clock::now() };
	unsigned int configs = 0;

	if (! std::filesystem::exists( configPath ) ) {
		Log_Error( "Depot config at `{}` does not exist!", configPath );
		return 1;
	}

	KV1 configKeyvalues{ sourcepp::fs::readFileText( configPath ) };
	if ( configKeyvalues.hasChild( "DepotBuildConfig" ) ) {
		Log_Error( R"(Depot config at `{}` has root key "DepotBuildConfig". Use the config with the root key "AppBuild" instead.)", configPath );
		return 1;
	}

	const auto& appBuildConfig = configKeyvalues[ "AppBuild" ];
	if ( appBuildConfig.isInvalid() ) {
		Log_Error( "Depot config at `{}` is invalid!", configPath );
		return 1;
	}

	auto contentRoot{ std::filesystem::path{ configPath }.parent_path() / appBuildConfig[ "ContentRoot" ].getValue() };
	const auto& depots = appBuildConfig[ "Depots" ];
	for ( const auto& depot : depots.getChildren() ) {
		if ( std::find( depotIDs.begin(), depotIDs.end(), depot.getKey() ) == depotIDs.end() ) {
			continue;
		}

		const auto createFromSteamDepotConfig{ [ &configPath, &indexLocation, skipArchives, &fileExcludes, &fileIncludes, &archiveExcludes, &archiveIncludes, &contentRoot ]( const auto& depotBuildConfig ) {
			std::vector<std::string> exclusionRegexes;
			exclusionRegexes.insert( exclusionRegexes.end(), fileExcludes.begin(), fileExcludes.end() );
			for ( int i = 0; i < depotBuildConfig.getChildCount( "FileExclusion" ); i++ ) {
				std::string exclusion{ depotBuildConfig( "FileExclusion", i ).getValue() };
				sourcepp::string::normalizeSlashes( exclusion );
				if ( exclusion.starts_with( "./" ) )
					exclusion = exclusion.substr(2);

				exclusionRegexes.emplace_back( globToRegex( exclusion ) );
			}

			std::vector<std::string> inclusionRegexes{ fileIncludes };
			for ( int i = 0; i < depotBuildConfig.getChildCount( "FileMapping" ); i++ ) {
				std::string inclusion{ depotBuildConfig( "FileMapping", i )[ "LocalPath" ].getValue() };
				sourcepp::string::normalizeSlashes( inclusion );
				if ( inclusion.starts_with( "./" ) )
					inclusion = inclusion.substr(2);

				inclusionRegexes.emplace_back( globToRegex( inclusion ) );
			}

			createFromRoot(
				depotBuildConfig.hasChild( "ContentRoot" ) ? ( std::filesystem::path{ configPath }.parent_path() / depotBuildConfig[ "ContentRoot" ].getValue() ).string() : contentRoot.string(),
				indexLocation,
				skipArchives,
				exclusionRegexes,
				inclusionRegexes,
				archiveExcludes,
				archiveIncludes
			);
		} };

		if ( depot.getChildCount() > 0 ) {
			createFromSteamDepotConfig( depot );
		} else {
			auto depotPath{ ( std::filesystem::path{ configPath }.parent_path() / depot.getValue() ).string() };
			if ( !std::filesystem::exists( depotPath ) ) {
				Log_Error( "Failed to load depot with ID `{}`: could not find depot config file at `{}`!", depot.getKey(), depotPath );
				continue;
			}

			KV1 depotBuildConfigKeyvalues{ sourcepp::fs::readFileText( depotPath ) };

			const auto& depotBuildConfig = depotBuildConfigKeyvalues[ "DepotBuildConfig" ];
			if ( depotBuildConfig.isInvalid() ) {
				Log_Error( "Depot config with ID `{}` at `{}` is invalid!", depot.getKey(), depotPath );
				continue;
			}

			createFromSteamDepotConfig( depotBuildConfig );
		}

		Log_Info( "Finished processing depot with ID `{}`.", depot.getKey() );
		configs++;
	}

	Log_Info( "Finished processing {} depot configs in {}.", configs, std::chrono::duration_cast<std::chrono::seconds>( std::chrono::high_resolution_clock::now() - start ) );
	return 0;
}

static auto enterVPK( std::ofstream& writer, std::string_view vpkPath, std::string_view vpkPathRel, const std::vector<std::regex>& excludes, const std::vector<std::regex>& includes, unsigned int& count ) -> bool {
	using namespace vpkpp;

	const auto vpk = VPK::open( std::string{ vpkPath } );
	if (! vpk ) {
		return false;
	}

	vpk->runForAllEntries( [ &writer, &vpkPath, &vpkPathRel, &excludes, &includes, &count, &vpk ]( const std::string& path, const Entry& entry ) {
		if ( !excludes.empty() && matchPath( path, excludes) ) {
			return;
		}

		if ( !includes.empty() && !matchPath( path, includes ) ) {
			return;
		}

		auto entryData{ vpk->readEntry( path ) };
		if (! entryData ) {
			Log_Error( "Failed to open file: `{}/{}`", vpkPath, path );
			return;
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
		writer << fmt::format( "{}\xFF{}\xFF{}\xFF{}\xFF{}\xFF\xFD", vpkPathRel, path, entryData->size(), sha1HashStr, crc32HashStr );
		Log_Verbose( "Processed file `{}/{}`", vpkPath, path );
		count += 1;
	} );

	return true;
}

static auto buildRegexCollection( const std::vector<std::string>& regexStrings, std::string_view collectionType ) -> std::vector<std::regex> {
	std::vector<std::regex> collection{};

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

static auto globToRegex( std::string_view glob ) -> std::string {
	static constexpr std::string_view SPECIAL_CHARS{ R"(.+^$()[]{}|-+:'"<>\#&!)" };

	std::string out;
	for ( char c : glob ) {
		if ( SPECIAL_CHARS.find( c ) != std::string_view::npos ) {
			out += '\\';
			out += c;
		} else if ( c == '?' ) {
			out += '.';
		} else if ( c == '*' ) {
			out += ".*?";
		} else {
			out += c;
		}
	}

	return out;
}
