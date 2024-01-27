//
// Created by ENDERZOMBI102 on 14/10/2023.
//
#include <vector>
#include <filesystem>
#include <array>
#include <argumentum/argparse.h>
#include <fmt/format.h>

#include "create.hpp"
#include "output/CsvOutput.hpp"
#include "output/JsonOutput.hpp"
#include "output/Output.hpp"
#include "output/SimpleOutput.hpp"
#include "verify.hpp"

// the index file is encoded as `Rows-of-String-Values`
// correct the index path with os
#if defined( _WIN32 )
	const auto INDEX_PATH = "bin/win64/index.rsv";
	const auto BIN_PATH = "bin/win64/";
#else
	const auto INDEX_PATH = "bin/linux64/index.rsv";
	const auto BIN_PATH = "bin/linux64";
#endif


auto main( int argc, char* argv[] ) -> int {
	std::string defaultRoot;
	{
		auto dir = std::filesystem::current_path();
		if ( dir.string().ends_with( BIN_PATH ) ) {
			// running directly from binaries directory, set root to ../..
			defaultRoot = dir.parent_path().parent_path().string();
		} else if ( dir.filename() == "bin/" ) {
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
		.description( "A tool used to verify a game's install." );

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
		.help( "GLOB pattern(s) to exclude when creating the index." )
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
		.choices( { "simple", "json", "csv" } )
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

	Output* output;
	switch ( format.at(0) ) {
		case 's':
			output = new SimpleOutput{};
			break;
		case 'j':
			output = new JsonOutput{};
			break;
		case 'c':
			output = new CsvOutput{};
			break;
		default:
			fmt::println( stderr, "Invalid `--format` argument: `{}`", format );
			return 1;
	}
	int ret;

	// `tee`, can be useful to track down a failure
	FILE* file{ nullptr };
	if ( tee )
		file = fopen( "./verifier.log", "w" );

	output->init( file );
	// `$basename started at $time` message
	{
		auto now{ std::time( nullptr ) };
		auto local{ std::localtime( &now ) };
		const auto line = fmt::format( "`{}` started at {}:{}:{}", programFile.string(), local->tm_hour, local->tm_min, local->tm_sec );
		output->write( OutputKind::Info, line );
	}
	if ( newIndex ) {
		// stuff we ignore during the building of the index, the "standard" useless stuff is hardcoded
		std::vector<std::string> ignored{
			"sdk_content/**/*.*",
			"hammer/autosave/*.*",
			"**/index.rsv",
		};
		excludes.insert( excludes.end(), ignored.begin(), ignored.end() );

		ret = create( root, indexLocation, excludes, overwrite, output );
	} else {
		// warn about stuff which shouldn't be here, don't use the `output::report` as this is a negligible user error
		if ( overwrite )
			fmt::println( stderr, "WARN: current action doesn't support `--overwrite`, please remove it." );
		if (! excludes.empty() )
			fmt::println( stderr, "WARN: current action doesn't support `--exclude`, please remove it." );

		ret = verify( root, indexLocation, output );
	}
	output->end();
	if ( file )
		fclose( file );

	return ret;
}
