//
// Created by ENDERZOMBI102 on 14/10/2023.
//
#include <fmt/format.h>

#include "SimpleOutput.hpp"

auto SimpleOutput::write( const OutputKind kind, const std::string_view message ) -> void {
	switch ( kind ) {
		case OutputKind::Info:
			fmt::println( stdout, "Info: {}", message );
			break;
		case OutputKind::Warn:
			fmt::println( stderr, "Warn: {}", message );
			break;
		case OutputKind::Error:
			fmt::println( stderr, "Error: {}", message );
			break;
	}
}

auto SimpleOutput::report( const std::string_view file, const std::string_view message, const std::string_view got, const std::string_view expected ) -> void {
	fmt::println( stderr, "In file `{}`: {}", file, message );
}
