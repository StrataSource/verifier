//
// Created by ENDERZOMBI102 on 14/10/2023.
//
#include <fmt/format.h>
#include <string_view>

#include "SimpleOutput.hpp"

auto SimpleOutput::write( const OutputKind kind, const std::string_view message ) const -> void {
	std::string line;
	FILE* where;
	switch ( kind ) {
		case OutputKind::Info:
			line = fmt::format( "Info: {}", message );
			where = stdout;
			break;
		case OutputKind::Warn:
			line = fmt::format( "Warn: {}", message );
			where = stderr;
			break;
		case OutputKind::Error:
			line = fmt::format( "Error: {}", message );
			where = stderr;
			break;
	}

	std::fprintf( where, "%s\n", line.c_str() );
	if ( this->teeFile )
		std::fprintf( this->teeFile, "%s\n", line.c_str() );
}

auto SimpleOutput::report( const std::string_view file, const std::string_view message, const std::string_view got, const std::string_view expected ) const -> void {
	const auto line{ fmt::format( "In file `{}`: {}", file, message ) };

	std::fprintf( stderr, "%s\n", line.c_str() );
	if ( this->teeFile )
		std::fprintf( this->teeFile, "%s\n", line.c_str() );
}
