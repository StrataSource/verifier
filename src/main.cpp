//
// Created by ENDERZOMBI102 on 14/10/2023.
//
#include <array>
#include <filesystem>
#include <memory>
#include <vector>

#include <argumentum/argparse.h>

#include "create.hpp"
#include "log.hpp"
#include "verify.hpp"

// the index file is encoded as `Rows-of-String-Values`
// correct the index path with os
#if defined( _WIN32 )
	const auto INDEX_PATH = "bin/win64/verifier_index.rsv";
	const auto BIN_OS_DIR = "win64";
#else
	const auto INDEX_PATH = "bin/linux64/verifier_index.rsv";
	const auto BIN_OS_DIR = "linux64";
#endif

auto main( int argc, char* argv[] ) -> int {
	std::string defaultRoot;
	{
		auto dir = std::filesystem::current_path();
		if ( dir.filename() == BIN_OS_DIR && dir.parent_path().filename() == "bin" ) {
			// running directly from binaries directory, set root to ../..
			defaultRoot = dir.parent_path().parent_path().string();
		} else if ( dir.filename() == "bin" ) {
			defaultRoot = dir.parent_path().string();
		} else {
			defaultRoot = dir.string();
		}
	}

	bool newIndex{ false };
	std::string root;
	bool skipArchives{ false };
	std::vector<std::string> fileExcludes;
	std::vector<std::string> fileIncludes;
	std::vector<std::string> archiveExcludes;
	std::vector<std::string> archiveIncludes;
	std::string steamDepotConfig;
	std::vector<std::string> steamDepotIDs;
	std::string indexLocation;
	bool overwrite{ false };
	const auto programFile{ std::filesystem::path( argv[ 0 ] ).filename() };

	argumentum::argument_parser parser{};
	parser.config()
		.program( programFile.string() )
		.description( "A tool used to verify a game's install. v" VERIFIER_VERSION );

	auto params = parser.params();
	params.add_parameter( newIndex, "--new-index" )
		.help( "Creates a new index file." )
		.metavar( "new-index" )
		.absent( false );
	params.add_parameter( root, "--root" )
		.help( "The engine root directory." )
		.metavar( "root" )
		.maxargs( 1 )
		.absent( defaultRoot );
	params.add_parameter( skipArchives, "--skip-archives" )
		.help( "Don't parse files stored in VPKs, parse the entire VPK instead." )
		.metavar( "skip-archives" )
		.absent( false );
	params.add_parameter( fileExcludes, "--exclude", "-e" )
		.help( "RegExp pattern(s) to exclude files when creating the index." )
		.metavar( "excluded" )
		.minargs( 1 );
	params.add_parameter( fileIncludes, "--include" )
		.help( "RegExp pattern(s) to include files when creating the index. If not present, all files not matching an exclusion will be included.")
		.metavar( "included" );
	params.add_parameter( archiveExcludes, "--archive-exclude", "-E" )
		.help( "RegExp pattern(s) to exclude files inside VPKs when creating the index." )
		.metavar( "archive-excluded" )
		.minargs( 1 );
	params.add_parameter( archiveIncludes, "--archive-include" )
		.help( "RegExp pattern(s) to include files inside VPKs when creating the index. If not present, all files inside VPKs not matching an exclusion will be included." )
		.metavar( "archive-included" );
	params.add_parameter( steamDepotConfig, "--steam-depot-config" )
		.help( "Use a Steam depot configuration file to include/exclude content. Pair this option with `--steam-depot-ids`." )
		.metavar( "steam-depot-config" )
		.maxargs( 1 );
	params.add_parameter( steamDepotIDs, "--steam-depot-ids" )
		.help( "The Steam depot IDs to include the content of. These should correspond with keys in the depots section of the Steam depot configuration file. Pair this option with `--steam-depot-config`." )
		.metavar( "steam-depot-ids" );
	params.add_parameter( indexLocation, "--index", "-i" )
		.help( "The index file to use." )
		.metavar( "index-loc" )
		.maxargs( 1 )
		.absent( INDEX_PATH );
	params.add_parameter( overwrite, "--overwrite" )
		.help( "Do not ask for confirmation for overwriting an existing index." )
		.metavar( "overwrite" );
	params.add_parameter( g_bUIReportMode, "--ui-report" )
		.help( "Use UI report mode logging." )
		.metavar( "ui-report" );

	if (! parser.parse_args( argc, argv, 1 ) ) {
		return 1;
	}

	// `$basename started at $time` message
	{
		auto now{ std::time( nullptr ) };
#ifndef _WIN32
		auto localPtr{ std::localtime( &now ) };
#else
		tm local{};
		localtime_s( &local, &now );
		auto* localPtr{ &local };
#endif
		Log_Info( "`{}` started at {:02d}:{:02d}:{:02d}", programFile.string(), localPtr->tm_hour, localPtr->tm_min, localPtr->tm_sec );
	}

	if ( newIndex ) {
		if ( const auto indexPath{ std::filesystem::path{ root } / indexLocation }; std::filesystem::exists( indexPath ) ) {
			if (! overwrite ) {
				Log_Error( "Index file `{}` already exists, do you want to overwrite it? (y/N)", indexPath.string() );
				std::string input;
				std::cin >> input;
				if ( input != "y" && input != "Y" ) {
					Log_Info( "Aborting." );
					return 1;
				}
			} else {
				Log_Warn( "Index file `{}` already exists, it will be overwritten.", indexPath.string() );
			}
			// clear contents
			std::ofstream writer{ indexPath, std::ios::out | std::ios::trunc };
		}

		// stuff we ignore during the building of the index, the "standard" useless stuff is hardcoded
		fileExcludes.emplace_back( "sdk_content.*" );
		fileExcludes.emplace_back( ".*\\.vmf_autosave.*" );
		fileExcludes.emplace_back( ".*\\.vmx" );
		fileExcludes.emplace_back( ".*\\.log" );
		fileExcludes.emplace_back( ".*verifier_index\\.rsv" );

		// if we're reading the contents of archives, numbered VPKs should not be considered
		if (! skipArchives ) {
			fileExcludes.emplace_back( R"(.*_[0-9][0-9][0-9]\.vpk)" );
		} else {
			if (! archiveExcludes.empty() )
				Log_Warn( "The current action doesn't support `--archive-exclude`, it will be ignored." );
			if (! archiveIncludes.empty() )
				Log_Warn( "The current action doesn't support `--archive-include`, it will be ignored." );
		}

		// create from a steam depot config
		if ( !steamDepotConfig.empty() || !steamDepotIDs.empty() ) {
			if ( steamDepotConfig.empty() && !steamDepotIDs.empty() ) {
				Log_Warn( "`--steam-depot-config` must be set when `--steam-depot-ids` is used." );
				return 1;
			}
			if ( !steamDepotConfig.empty() && steamDepotIDs.empty() ) {
				Log_Warn( "`--steam-depot-ids` must be set when `--steam-depot-config` is used." );
				return 1;
			}

			return createFromSteamDepotConfigs( steamDepotConfig, steamDepotIDs, indexLocation, skipArchives, fileExcludes, fileIncludes, archiveExcludes, archiveIncludes );
		}

		return createFromRoot( root, indexLocation, skipArchives, fileExcludes, fileIncludes, archiveExcludes, archiveIncludes );
	}

	if ( skipArchives )
		Log_Warn( "The current action doesn't support `--skip-archives`, it will be ignored." );
	if (! fileExcludes.empty() )
		Log_Warn( "The current action doesn't support `--exclude`, it will be ignored." );
	if (! fileIncludes.empty() )
		Log_Warn( "The current action doesn't support `--include`, it will be ignored." );
	if (! archiveExcludes.empty() )
		Log_Warn( "The current action doesn't support `--archive-exclude`, it will be ignored." );
	if (! archiveIncludes.empty() )
		Log_Warn( "The current action doesn't support `--archive-include`, it will be ignored." );
	if (! steamDepotConfig.empty() )
		Log_Warn( "The current action doesn't support `--steam-depot-config`, it will be ignored." );
	if (! steamDepotIDs.empty() )
		Log_Warn( "The current action doesn't support `--steam-depot-ids`, it will be ignored." );
	if ( overwrite )
		Log_Warn( "The current action doesn't support `--overwrite`, it will be ignored." );

	return verify( root, indexLocation );
}
