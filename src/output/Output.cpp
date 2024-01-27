//
// Created by ENDERZOMBI102 on 15/10/2023.
//
#include "Output.hpp"


auto toString( const OutputKind kind ) -> std::string_view {
	switch ( kind ) {
		case OutputKind::Info: return "info";
		case OutputKind::Warn: return "warn";
		case OutputKind::Error: return "error";
	}
	return {};
}
