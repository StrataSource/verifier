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
	std::string root{};
	bool parseArchives{ false };
	std::vector<std::string> excludes{};
	std::string indexLocation{};
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
	params.add_parameter( parseArchives, "--parse-archives" )
		.help( "Parse files stored in VPKs." )
		.metavar( "parse-archives" )
		.absent( false );
	params.add_parameter( excludes, "--exclude", "-e" )
		.help( "RegExp pattern(s) to exclude files when creating the index." )
		.metavar( "excluded" )
		.minargs( 1 );
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

	int ret;
	if ( newIndex ) {
		// stuff we ignore during the building of the index, the "standard" useless stuff is hardcoded
		excludes.emplace_back( "sdk_content.*" );
		excludes.emplace_back( ".*\\.vmf_autosave.*" );
		excludes.emplace_back( ".*\\.vmx" );
		excludes.emplace_back( ".*\\.log" );
		excludes.emplace_back( ".*verifier_index\\.rsv" );
		ret = create( root, indexLocation, parseArchives, excludes, overwrite );
	} else {
		if ( overwrite )
			Log_Error( "current action doesn't support `--overwrite`, please remove it." );
		if (! excludes.empty() )
			Log_Error( "current action doesn't support `--exclude`, please remove it." );

		ret = verify( root, indexLocation );
	}

	return ret;
}
