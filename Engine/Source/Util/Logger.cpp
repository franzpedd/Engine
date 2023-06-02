#include "Logger.h"

#include "Platform/Platform.h"

#include <cstdarg>
#include <sstream>

#define MAX_OUTPUT_CHARS 1024

namespace Cosmos
{
	//Trace = 0, Info, Debug, Warning, Error, Fatal
	void ThrowToConsole(LogLevel level, const char* message, ...)
	{
		const char* strLevels[] = {"[Trace]: ", "[Info]: ", "[Debug]: ", "[Warning]: ", "[Error]: ", "[Fatal]: "};
		char outputMessage[MAX_OUTPUT_CHARS] = {};
		
		va_list args;
		va_start(args, message);
		vsnprintf(outputMessage, MAX_OUTPUT_CHARS, message, args);
		va_end(args);

		std::ostringstream oss;
		oss << strLevels[level] << outputMessage << std::endl;
	
		OutputMessageToConsole(oss.str().c_str(), (u8)level);
	}

	void ThrowAssertion(const char* expression, const char* message, const char* file, i32 line)
	{
		std::ostringstream oss;
		oss << "[Assertion: " << expression << "]";
		oss << "[Message: " << message << "]";
		oss << "[File: " << file << "]";
		oss << "[Line: " << line << "]";

		OutputErrorToConsole(oss.str().c_str(), (u8)LogLevel::Fatal);
	}
}