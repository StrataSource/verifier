//
// Created by ENDERZOMBI102 on 14/10/2023.
//
#include <array>
#include <filesystem>
#include <memory>
#include <vector>
#include <argumentum/argparse.h>
#include <fmt/format.h>

#include "create.hpp"
#include "output/CsvOutput.hpp"
#include "output/RsvOutput.hpp"
#include "output/JsonOutput.hpp"
#include "output/Output.hpp"
#include "output/SimpleOutput.hpp"
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
	std::vector<std::string> excludes{};
	std::string indexLocation{};
	std::string format{};
	bool overwrite{ false };
	bool tee{ false };
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
		.setLongName("--root")
		.metavar( "root" )
		.maxargs( 1 )
		.absent( defaultRoot );
	params.add_parameter( excludes, "--exclude", "-e" )
		.help( "RegExp pattern(s) to exclude files when creating the index." )
		.metavar( "excluded" )
		.minargs( 1 );
	params.add_parameter( indexLocation, "--index", "-i" )
		.help( "The index file to use." )
		.metavar( "index-loc" )
		.maxargs( 1 )
		.absent( INDEX_PATH );
	params.add_parameter( format, "--format", "-f" )
		.help( "Output format. [simple, json, csv] default: `simple`" )
		.metavar( "format" )
		.choices( { "simple", "json", "csv", "rsv" } )
		.maxargs( 1 )
		.absent( "simple" );
	params.add_parameter( overwrite, "--overwrite" )
		.help( "Do not ask for confirmation for overwriting an existing index." )
		.metavar( "overwrite" );
	params.add_parameter( tee, "--tee" )
		.help( "Also write the program's output to `$workDir/verifier.log`." )
		.metavar( "tee" )
		.absent( false );

	if (! parser.parse_args( argc, argv, 1 ) ) {
		return 1;
	}

	std::unique_ptr<Output> output;
	switch ( format.at(0) ) {
		case 's':
			output = std::make_unique<SimpleOutput>();
			break;
		case 'j':
			output = std::make_unique<JsonOutput>();
			break;
		case 'c':
			output = std::make_unique<CsvOutput>();
			break;
		case 'r':
			output = std::make_unique<RsvOutput>();
			break;
		default:
			fmt::println( stderr, "Invalid `--format` argument: `{}`", format );
			return 1;
	}
	int ret;

	// `tee`, can be useful to track down a failure
	FILE* file{ nullptr };
	if ( tee ) {
#ifndef _WIN32
		file = std::fopen( "./verifier.log", "w" );
#else
		fopen_s( &file, "./verifier.log", "w" );
#endif
	}

	output->init( file );
	// `$basename started at $time` message
	{
		auto now{ std::time( nullptr ) };
#ifndef _WIN32
		auto local{ std::localtime( &now ) };
		const auto line = fmt::format( "`{}` started at {:02d}:{:02d}:{:02d}", programFile.string(), local->tm_hour, local->tm_min, local->tm_sec );
#else
		tm local{};
		localtime_s( &local, &now );
		const auto line = fmt::format( "`{}` started at {:02d}:{:02d}:{:02d}", programFile.string(), local.tm_hour, local.tm_min, local.tm_sec );
#endif
		output->write( OutputKind::Info, line );
	}
	if ( newIndex ) {
		// stuff we ignore during the building of the index, the "standard" useless stuff is hardcoded
		excludes.emplace_back( "sdk_content.*" );
		excludes.emplace_back( ".*\\.vmf_autosave.*" );
		excludes.emplace_back( ".*\\.vmx" );
		excludes.emplace_back( ".*\\.log" );
		excludes.emplace_back( ".*verifier_index\\.rsv" );
		ret = create( root, indexLocation, excludes, overwrite, output.get() );
	} else {
		// warn about stuff which shouldn't be here, don't use the `output::report` as this is a negligible user error
		if ( overwrite )
			fmt::println( stderr, "WARN: current action doesn't support `--overwrite`, please remove it." );
		if (! excludes.empty() )
			fmt::println( stderr, "WARN: current action doesn't support `--exclude`, please remove it." );

		ret = verify( root, indexLocation, output.get() );
	}
	if ( file )
		fclose( file );

	return ret;
}
