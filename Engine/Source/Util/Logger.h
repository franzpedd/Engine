#pragma once

#include <ctime>
#include <cstdarg>
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
		~Logger() 
		{
			printf("%s", m_Output.str().c_str());
		}
	
		// logs to a ostringstream object
		template<class T>
		inline Logger& operator<<(const T& other)
		{
			m_Output << other << std::endl;
			return *this;
		}
	
	public:
	
		// outputs a message to os's terminal
		static inline void ToTerminal(Severity severity, const char* file, int line, const char* msg, ...)
		{
			char buffer[LOG_MAX_SIZE];
			va_list args;
			va_start(args, msg);
			vsnprintf(buffer, LOG_MAX_SIZE, msg, args);
			va_end(args);

			time_t ttime = time(0);
			tm* local_time = localtime(&ttime);

			std::stringstream oss;
			oss << "[" << local_time->tm_mday << "/" << 1 + local_time->tm_mon << "/" << 1900 + local_time->tm_year;
			oss << " - " << local_time->tm_hour << ":" << local_time->tm_min << ":" << local_time->tm_sec << "]";
			oss << "[" << file << " - " << line << "]";
			oss << "[" << SeverityToConstChar(severity) << "]";
			oss << ": " << buffer;

			printf("%s\n", oss.str().c_str());
		}
	
		// outputs a message to a file
		static inline void ToFile(Severity severity, const char* path, const char* file, int line, const char* msg, ...)
		{
			FILE* f = fopen(path, "a+");

			char buffer[LOG_MAX_SIZE];
			va_list args;
			va_start(args, msg);
			vsnprintf(buffer, LOG_MAX_SIZE, msg, args);
			va_end(args);

			time_t ttime = time(0);
			tm* local_time = localtime(&ttime);

			std::stringstream oss;
			oss << "[" << local_time->tm_mday << "/" << 1 + local_time->tm_mon << "/" << 1900 + local_time->tm_year;
			oss << " - " << local_time->tm_hour << ":" << local_time->tm_min << ":" << local_time->tm_sec << "]";
			oss << "[" << file << " - " << line << "]";
			oss << "[" << SeverityToConstChar(severity) << "]";
			oss << ": " << buffer;

			fprintf(f, "%s\n", oss.str().c_str());

			fclose(f);
		}
	
		// translates severity level to readable text
		static inline const char* SeverityToConstChar(Severity severity)
		{
			switch (severity)
			{
			case Severity::Trace: return "Trace";
			case Severity::Info: return "Info";
			case Severity::Warn: return "Warning";
			case Severity::Error: return "Error";
			case Severity::Assert: return "Assertion";
			}

			return "Undefined Severity Level";
		}
	
	private:
	
		std::ostringstream m_Output;
	};
}

// macros to facilitate using logging (only enabled on debug)
#if defined(ENGINE_DEBUG) || defined(EDITOR_DEBUG)

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

#define VK_ASSERT(fn, ...)																					\
{																											\
	VkResult res = (fn);																					\
	if (res != VK_SUCCESS)																					\
	{																										\
		Cosmos::Logger::ToTerminal(Cosmos::Logger::Severity::Assert, __FILE__, __LINE__, __VA_ARGS__);		\
		std::abort();																						\
	}																										\
}

#else

#define LOG_TO_FILE(severity, filepath, ...);
#define LOG_TO_TERMINAL(severity, ...);
#define LOG_ASSERT(x, ...);
#define VK_ASSERT(fn, ...);	

#endif