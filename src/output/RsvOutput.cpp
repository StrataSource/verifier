//
// Created by ENDERZOMBI102 on 1/29/24.
//
#include <fmt/format.h>
#include <string_view>

#include "RsvOutput.hpp"


auto RsvOutput::init( FILE* teeFile ) -> void {
	Output::init( teeFile );
	fmt::println( "type\xFF" "context\xFFmessage\xFFgot?\xFF" "expected?\xFF\xFD" );
	if ( this->teeFile )
		fmt::println( this->teeFile, "type\xFF" "context\xFFmessage\xFFgot?\xFF" "expected?\xFF\xFD" );
}

auto RsvOutput::write( OutputKind kind, std::string_view message ) const -> void {
	const auto line{ fmt::format( "message\xFF{}\xFF{}\xFF\xFE\xFF\xFE\xFF\xFD", toString( kind ), message ) };
	std::printf( "%s", line.c_str() );
	if ( this->teeFile )
		std::fprintf( this->teeFile, "%s", line.c_str() );
}

auto RsvOutput::report( std::string_view file, std::string_view message, std::string_view got, std::string_view expected ) const -> void {
	const  auto line{ fmt::format( "report\xFF{}\xFF{}\xFF{}\xFF{}\xFF\xFD", file, message, got, expected ) };
	std::printf( "%s", line.c_str() );
	if ( this->teeFile )
		std::fprintf( this->teeFile, "%s", line.c_str() );
}