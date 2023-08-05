#pragma once

#include <sstream>

#define LOG_MAX_SIZE 1024

namespace Cosmos
{
	class Logger
	{
	public:

		typedef enum Severity
		{
			Trace = 0,
			Info,
			Warn,
			Error,
			Assert
		} Severity;

	public:
	
		// constructor
		Logger() = default;
	
		// destructor
		~Logger();
	
		// logs to a ostringstream object
		template<class T>
		inline Logger& operator<<(const T& other)
		{
			m_Output << other << std::endl;
			return *this;
		}
	
	public:
	
		// outputs a message to os's terminal
		static void ToTerminal(Severity severity, const char* file, int line, const char* msg, ...);
	
		// outputs a message to a file
		static void ToFile(Severity severity, const char* path, const char* file, int line, const char* msg, ...);
	
		// translates severity level to readable text
		static const char* SeverityToConstChar(Severity severity);
	
	private:
	
		std::ostringstream m_Output;
	
	};
}

// macros to facilitate using logging

#define LOG_TO_FILE(severity, filepath, ...)										\
{																					\
	Cosmos::Logger::ToFile(severity, filepath, __FILE__, __LINE__, __VA_ARGS__);	\
	if(severity == Cosmos::Logger::Severity::Assert) std::abort();					\
}

#define LOG_TO_TERMINAL(severity, ...)										\
{																			\
	Cosmos::Logger::ToTerminal(severity, __FILE__, __LINE__, __VA_ARGS__);	\
	if (severity == Cosmos::Logger::Severity::Assert) std::abort();			\
}
	
#define LOG_ASSERT(x, ...)																				\
{																										\
	if(!(x))																							\
	{																									\
		Cosmos::Logger::ToTerminal(Cosmos::Logger::Severity::Assert, __FILE__, __LINE__, __VA_ARGS__);	\
		std::abort();																					\
	}																									\
}