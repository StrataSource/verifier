#pragma once

#include <fmt/format.h>
#include <fmt/chrono.h>

enum class LogSeverity
{
	Info,
	Warn,
	Error,
};

// Base logging function
auto Log_Message( LogSeverity severity, std::string_view message ) -> void;

// Report
extern bool g_bUIReportMode;
auto Log_Report( std::string_view file, std::string_view message, std::string_view got, std::string_view expected ) -> void;

// Logging helpers
template <typename... Ts>
inline auto Log_Info( const fmt::format_string<Ts...> fmt, Ts&&... args ) -> void
{
	Log_Message( LogSeverity::Info, fmt::format( fmt, std::forward<Ts>( args )... ) );
}

template <typename... Ts>
inline auto Log_Warn( const fmt::format_string<Ts...> fmt, Ts&&... args ) -> void
{
	Log_Message( LogSeverity::Warn, fmt::format( fmt, std::forward<Ts>( args )... ) );
}

template <typename... Ts>
inline auto Log_Error( const fmt::format_string<Ts...> fmt, Ts&&... args ) -> void
{
	Log_Message( LogSeverity::Error, fmt::format( fmt, std::forward<Ts>(args)... ) );
}
