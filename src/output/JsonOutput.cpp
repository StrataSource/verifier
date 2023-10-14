//
// Created by ENDERZOMBI102 on 14/10/2023.
//
#include <fmt/format.h>

#include "JsonOutput.hpp"

auto JsonOutput::write( const OutputKind kind, const std::string_view message ) -> void {
	fmt::println(
		R"({{"type":"message","kind":"{}","message":"{}"}})",
		toString( kind ), message
	);
}

auto JsonOutput::report( const std::string_view file, const std::string_view message, const std::string_view got, const std::string_view expected ) -> void {
	fmt::println(
		R"({{"type":"report","file":"{}","message":"{}","got":"{}","expected":"{}"}})",
		file, message, got, expected
	);
}
