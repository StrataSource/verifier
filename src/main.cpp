//
// Created by ENDERZOMBI102 on 14/10/2023.
//
#include <vector>
#include <filesystem>
#include <array>
#include <argumentum/argparse.h>
#include <fmt/format.h>
#include <fmt/format.h>

#include "create.hpp"
#include "output/CsvOutput.hpp"
#include "output/JsonOutput.hpp"
#include "output/Output.hpp"
#include "output/SimpleOutput.hpp"
#include "verify.hpp"

// correct the index path with os
#if defined( _WIN32 )
	const auto INDEX_PATH = L"bin/win64/index.csv";
	const auto BIN_PATH = L"bin/win64/";
#else
	const auto INDEX_PATH = L"bin/linux64/index.csv";
	const auto BIN_PATH = L"bin/linux64";
#endif


auto main( int argc, char* argv[] ) -> int {
	std::wstring defaultRoot;
	{
		auto dir = std::filesystem::current_path();
		if ( dir.wstring().ends_with( BIN_PATH ) ) {
			// running directly from binaries directory, set root to ../..
			defaultRoot = dir.parent_path().parent_path();
		} else if ( dir.filename() == L"bin/" ) {
			defaultRoot = dir.parent_path();
		} else {
			defaultRoot = dir;
		}
	}

	bool newIndex{ false };
	std::wstring root{};
	std::vector<std::wstring> excludes{};
	std::wstring indexLocation{};
	std::wstring format{};
	bool overwrite{ false };

	argumentum::argument_parser parser{};
	parser.config()
		.program( argv[0] )
		.description( "A tool used to verify a game's install" );

	auto params = parser.params();
	params.add_parameter( newIndex, "--new-index" )
		.help( "Creates a new index file" )
		.metavar( "new-index" )
		.absent( false );
	params.add_parameter( root, "--root" )
		.help( "The engine root directory" )
		.metavar( "root" )
		.absent( defaultRoot );
	params.add_parameter( excludes, "--exclude", "-e" )
		.help( "GLOB pattern(s) to exclude when creating the index" )
		.metavar( "excluded" )
		.minargs( 1 ); // TODO: Add check for --new-index
	params.add_parameter( indexLocation, "--index", "-i" )
		.help( "The index file to use" )
		.metavar( "index-loc" )
		.absent( INDEX_PATH );
	params.add_parameter( format, "--format", "-f" )
		.help( "Output format" )
		.metavar( "format" )
		.choices( { "simple", "json", "csv" } )
		.absent( L"simple" );
	params.add_parameter( overwrite, "--overwrite" )
		.help( "Do not ask for confirmation for overwriting an existing index" )
		.metavar( "overwrite" ); // TODO: Add check for --new-index

	if (! parser.parse_args( argc, argv, 1 ) )
		return 1;

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
			fmt::println( L"Invalid `--format` argument: `{}`", format );
			return 1;
	}
	int ret;

	output->init();
	if ( newIndex ) {
		std::vector<std::string> ignored{
			"sdk_content/**/*.*",
			"hammer/autosave/*.*",
			"**/index.csv",
		};
		excludes.insert( excludes.end(), ignored.begin(), ignored.end() );

		ret = create( root, indexLocation, excludes, overwrite, output );
	} else {
		ret = verify( root, indexLocation, output );
	}
	output->end();

	return ret;
}
