//
// Created by ENDERZOMBI102 on 14/10/2023.
//
#pragma once

#include "Output.hpp"

class CsvOutput : public Output {
public:
	CsvOutput() = default;
	CsvOutput( const CsvOutput& ) = delete;
	CsvOutput operator=( const CsvOutput& ) = delete;
public:
	auto init() -> void override;
	auto write( OutputKind kind, std::string_view message ) -> void override;
	auto report( std::string_view  file, std::string_view message, std::string_view got, std::string_view expected ) -> void override;
};
