//
// Created by ENDERZOMBI102 on 1/29/24.
//
#pragma once

#include "Output.hpp"

class RsvOutput : public Output {
public:
	RsvOutput() = default;
	RsvOutput( const RsvOutput& ) = delete;
	RsvOutput operator=( const RsvOutput& ) = delete;
public:
	auto init( FILE* teeFile ) -> void override;
	auto write( OutputKind kind, std::string_view message ) const -> void override;
	auto report( std::string_view  file, std::string_view message, std::string_view got, std::string_view expected ) const -> void override;
};
