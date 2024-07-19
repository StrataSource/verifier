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
void Log_Message( LogSeverity severity, const std::string_view message );

// Report
extern bool g_bUIReportMode;
void Log_Report( const std::string_view file, const std::string_view message, const std::string_view got, const std::string_view expected );

// Logging helpers
template <typename... Ts>
inline void Log_Info( const fmt::format_string<Ts...> fmt, Ts&&... args )
{
	Log_Message( LogSeverity::Info, fmt::format( fmt, std::forward<Ts>( args )... ) );
}

template <typename... Ts>
inline void Log_Warn( const fmt::format_string<Ts...> fmt, Ts&&... args )
{
	Log_Message( LogSeverity::Warn, fmt::format( fmt, std::forward<Ts>( args )... ) );
}

template <typename... Ts>
inline void Log_Error( const fmt::format_string<Ts...> fmt, Ts&&... args )
{
	Log_Message( LogSeverity::Error, fmt::format( fmt, std::forward<Ts>(args)... ) );
}

