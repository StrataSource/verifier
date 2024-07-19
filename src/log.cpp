#include "log.hpp"
#include <iostream>

bool g_bUIReportMode = false;

void Log_Message( LogSeverity severity, const std::string_view message )
{
	// Don't log in report only mode!
	const char* prefixes[] = {
		"Info",
		"Warn",
		"Error",
	};
	const char* prefix = prefixes[(int)severity];

	if ( g_bUIReportMode )
	{
		// Print for UI to read
		const auto line{ fmt::format( R"("message","{}","{}",,)", prefix, message ) };
		std::printf( "%s\n", line.c_str() );
	}
	else
	{
		// Standard print
		std::cout << prefix << ": " << message << "\n";
	}
}

void Log_Report( const std::string_view file, const std::string_view message, const std::string_view got, const std::string_view expected )
{
	if ( g_bUIReportMode )
	{
		const  auto line{ fmt::format( R"("report","{}","{}","{}","{}")", file, message, got, expected ) };
		std::printf( "%s\n", line.c_str() );
	}
	else
	{
		const auto line{ fmt::format( "In file `{}`: {}", file, message ) };
		std::fprintf( stderr, "%s\n", line.c_str() );
	}
}
