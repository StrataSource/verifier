//
// Created by ENDERZOMBI102 on 14/10/2023.
//
#include <fmt/format.h>
#include <string_view>

#include "JsonOutput.hpp"

auto JsonOutput::write( const OutputKind kind, const std::string_view message ) const -> void {
	const auto line{ fmt::format( R"({{"type":"message","kind":"{}","message":"{}"}})", toString( kind ), message ) };

	std::printf( "%s\n", line.c_str() );
	if ( this->teeFile )
		std::fprintf( this->teeFile, "%s\n", line.c_str() );
}

auto JsonOutput::report( const std::string_view file, const std::string_view message, const std::string_view got, const std::string_view expected ) const -> void {
	const auto line{ fmt::format( R"({{"type":"report","file":"{}","message":"{}","got":"{}","expected":"{}"}})", file, message, got, expected ) };

	std::printf( "%s\n", line.c_str() );
	if ( this->teeFile )
		std::fprintf( this->teeFile, "%s\n", line.c_str() );
}
