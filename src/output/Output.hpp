//
// Created by ENDERZOMBI102 on 14/10/2023.
//
#pragma once

#include <string_view>

enum class OutputKind {
	Info,
	Warn,
	Error
};

auto toString( OutputKind kind ) -> std::string_view;

class Output {
public:
	virtual ~Output() = default;
	virtual auto init( FILE* teeFile_ ) -> void {
		this->teeFile = teeFile_;
	}
	virtual auto write( OutputKind kind, std::string_view message ) const -> void = 0;
	virtual auto report( std::string_view  file, std::string_view message, std::string_view got, std::string_view expected ) const -> void = 0;
protected:
	FILE* teeFile{ nullptr };
};
