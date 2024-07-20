#include "log.hpp"

#include <cstdio>

bool g_bUIReportMode = false;

void Log_Message( LogSeverity severity, std::string_view message ) {
	// Don't log in report only mode!
	static constexpr std::string_view prefixes[] = {
		"Info",
		"Warn",
		"Error",
	};
	const auto prefix{ prefixes[ static_cast<int>( severity ) ] };

	if ( g_bUIReportMode ) {
		// Print for UI to read
		std::printf( R"("message","%s","%s",,)" "\n", prefix.data(), message.data() );
	} else {
		// Standard print
		std::printf("%s: %s\n", prefix.data(), message.data());
	}
}

void Log_Report( std::string_view file, std::string_view message, std::string_view got, std::string_view expected ) {
	if ( g_bUIReportMode ) {
		std::printf( R"("report","%s","%s","%s","%s")" "\n", file.data(), message.data(), got.data(), expected.data() );
	} else {
		std::fprintf( stderr, "In file `%s`: %s\n", file.data(), message.data() );
	}
}
