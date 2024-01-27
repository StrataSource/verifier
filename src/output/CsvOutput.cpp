//
// Created by ENDERZOMBI102 on 14/10/2023.
//
#include <fmt/format.h>
#include <string_view>

#include "CsvOutput.hpp"

auto CsvOutput::init( FILE* teeFile ) -> void {
	Output::init( teeFile );
	fmt::println( "type,context,message,got?,expected?" );
	if ( this->teeFile )
		fmt::println( this->teeFile, "type,context,message,got?,expected?\n" );
}

auto CsvOutput::write( OutputKind kind, std::string_view message ) const -> void {
	const auto line{ fmt::format( R"("message","{}","{}",,)", toString( kind ), message ) };
	std::printf( "%s\n", line.c_str() );
	if ( this->teeFile )
		std::fprintf( this->teeFile, "%s\n", line.c_str() );
}

auto CsvOutput::report( std::string_view file, std::string_view message, std::string_view got, std::string_view expected ) const -> void {
	const  auto line{ fmt::format( R"("report","{}","{}","{}","{}")", file, message, got, expected ) };
	std::printf( "%s\n", line.c_str() );
	if ( this->teeFile )
		std::fprintf( this->teeFile, "%s\n", line.c_str() );
}
