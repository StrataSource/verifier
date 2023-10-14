//
// Created by ENDERZOMBI102 on 14/10/2023.
//
#pragma once

#include "Output.hpp"

class SimpleOutput : public Output {
public:
	SimpleOutput() = default;
	SimpleOutput( const SimpleOutput& ) = delete;
	SimpleOutput operator=( const SimpleOutput& ) = delete;
public:
	auto write( OutputKind kind, std::string_view message ) -> void override;
	auto report( std::string_view file, std::string_view message, std::string_view got, std::string_view expected ) -> void override;
};
