#pragma once

#include <sstream>

#define LOG_MAX_SIZE 1024
#define LOG_MAX_ENTRIES_SIZE 20

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

		// returns the logger
		static Logger& Get();
	
		// logs to a ostringstream object
		template<class T>
		inline Logger& operator<<(const T& other)
		{
			mOutput << other << std::endl;
			return *this;
		}

		// sets/unsets the usage of external console
		inline void UseExternalConsole(bool value) { mExternalLogger = value; }

		// returns the stored console messages
		inline std::ostringstream& GetMessages() { return mConsoleMessage; }
	
	public:
	
		// outputs a message to os's terminal
		void ToTerminal(Severity severity, const char* file, int line, const char* msg, ...);
	
		// outputs a message to a file
		void ToFile(Severity severity, const char* path, const char* file, int line, const char* msg, ...);
	
		// translates severity level to readable text
		const char* SeverityToConstChar(Severity severity);
	
	private:
		
		bool mExternalLogger = false;
		std::ostringstream mOutput;
		std::ostringstream mConsoleMessage;
	};
}

// macros to facilitate using logging (only enabled on debug)
#if defined(ENGINE_DEBUG) || defined(EDITOR_DEBUG)

#define LOG_TO_FILE(severity, filepath, ...)										\
{																					\
	Cosmos::Logger::Get().ToFile(severity, filepath, __FILE__, __LINE__, __VA_ARGS__);	\
	if(severity == Cosmos::Logger::Severity::Assert) std::abort();					\
}

#define LOG_TO_TERMINAL(severity, ...)										\
{																			\
	Cosmos::Logger::Get().ToTerminal(severity, __FILE__, __LINE__, __VA_ARGS__);	\
	if (severity == Cosmos::Logger::Severity::Assert) std::abort();			\
}
	
#define LOG_ASSERT(x, ...)																						\
{																												\
	if(!(x))																									\
	{																											\
		Cosmos::Logger::Get().ToTerminal(Cosmos::Logger::Severity::Assert, __FILE__, __LINE__, __VA_ARGS__);	\
		std::abort();																							\
	}																											\
}

#define VK_ASSERT(fn, ...)																						\
{																												\
	VkResult res = (fn);																						\
	if (res != VK_SUCCESS)																						\
	{																											\
		Cosmos::Logger::Get().ToTerminal(Cosmos::Logger::Severity::Assert, __FILE__, __LINE__, __VA_ARGS__);	\
		std::abort();																							\
	}																											\
}

#else

#define LOG_TO_FILE(severity, filepath, ...);
#define LOG_TO_TERMINAL(severity, ...);
#define LOG_ASSERT(x, ...);
#define VK_ASSERT(fn, ...);	

#endif