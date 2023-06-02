#pragma once

#include "Core/Defines.h"

namespace Cosmos
{
	typedef enum LogLevel
	{
		Trace = 0, Info, Debug, Warning, Error, Fatal
	} LogLevel;

	// print's out to window console a message
	COSMOS_API void ThrowToConsole(LogLevel level, const char* message, ...);

	// prints out to window console an assertion
	COSMOS_API void ThrowAssertion(const char* expression, const char* message, const char* file, i32 line);
}

#define LOG_FATAL(message, ...) Cosmos::ThrowToConsole(Cosmos::LogLevel::Fatal, message, ##__VA_ARGS__);
#define LOG_ERROR(message, ...) Cosmos::ThrowToConsole(Cosmos::LogLevel::Error, message, ##__VA_ARGS__);
#define LOG_WARNING(message, ...) Cosmos::ThrowToConsole(Cosmos::LogLevel::Warning, message, ##__VA_ARGS__);

#if defined(ENGINE_DEBUG) || defined(RUNTIME_DEBUG)
	#define LOG_DEBUG(message, ...) Cosmos::ThrowToConsole(Cosmos::LogLevel::Debug, message, ##__VA_ARGS__);
	#define LOG_INFO(message, ...) Cosmos::ThrowToConsole(Cosmos::LogLevel::Info, message, ##__VA_ARGS__);
	#define LOG_TRACE(message, ...) Cosmos::ThrowToConsole(Cosmos::LogLevel::Trace, message, ##__VA_ARGS__);
#elif
	#define LOG_DEBUG(message, ...)
	#define LOG_INFO(message, ...)
	#define LOG_TRACE(message, ...)
#endif

#define LOG_ASSERT(expr, message) { if(expr) {} else { Cosmos::ThrowAssertion(#expr, message, __FILE__, __LINE__); debugBreak(); } }