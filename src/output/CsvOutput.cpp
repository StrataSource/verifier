//
// Created by ENDERZOMBI102 on 14/10/2023.
//
#include <fmt/format.h>

#include "CsvOutput.hpp"

auto CsvOutput::init() -> void {
	fmt::println( "type,context,message,got?,expected?" );
}

auto CsvOutput::write( OutputKind kind, std::string_view message ) -> void {
	fmt::println( R"("message","{}","{}")", toString( kind ), message );
}

auto CsvOutput::report( std::string_view file, std::string_view message, std::string_view got, std::string_view expected ) -> void {
	fmt::println( R"("report","{}","{}","{}","{}")", file, message, got, expected );
}
