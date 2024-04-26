#pragma once

#include <sstream>
#include <vector>

#define LOG_MAX_SIZE 1024
#define LOG_MAX_ENTRIES_SIZE 50

namespace Cosmos
{
	class Logger
	{
	public:

		typedef enum Severity
		{
			Trace = 0,
			Info,
			Todo,
			Warn,
			Error,
			Assert,

			MAX_SEVERITY
		} Severity;

		struct ConsoleMessage
		{
			Severity severity;
			std::string message;
		};

	public:
	
		// constructor
		Logger() = default;
	
		// destructor
		~Logger();

		// returns the logger
		static Logger& GetInstance();
	
		// logs to a ostringstream object
		template<class T>
		inline Logger& operator<<(const T& other)
		{
			mOutput << other << std::endl;
			return *this;
		}

		// returns the stored console messages
		inline std::vector<ConsoleMessage>& GetMessages() { return mConsoleMessages; }
	
	public:
	
		// outputs a message to os's terminal
		void ToTerminal(Severity severity, const char* file, int line, const char* msg, ...);
	
		// outputs a message to a file
		void ToFile(Severity severity, const char* path, const char* file, int line, const char* msg, ...);
	
		// translates severity level to readable text
		const char* SeverityToConstChar(Severity severity);
	
	private:
		
		std::ostringstream mOutput;
		std::vector<ConsoleMessage> mConsoleMessages;
	};
}

// macros to facilitate using logging (only enabled on debug)
#if defined(ENGINE_DEBUG) || defined(EDITOR_DEBUG) || defined(GAME_DEBUG)

#define LOG_TO_FILE(severity, filepath, ...)													\
{																								\
	Cosmos::Logger::GetInstance().ToFile(severity, filepath, __FILE__, __LINE__, __VA_ARGS__);	\
	if(severity == Cosmos::Logger::Severity::Assert) std::abort();								\
}

#define LOG_TO_TERMINAL(severity, ...)															\
{																								\
	Cosmos::Logger::GetInstance().ToTerminal(severity, __FILE__, __LINE__, __VA_ARGS__);		\
	if (severity == Cosmos::Logger::Severity::Assert) std::abort();								\
}

#else

#define LOG_TO_FILE(severity, filepath, ...);
#define LOG_TO_TERMINAL(severity, ...);

#endif

// always enable vulkan asserts and asserts
#if defined (ENGINE_RELEASE) || defined(EDITOR_RELEASE)	

#define LOG_ASSERT(x, ...)																								\
{																														\
	if(!(x))																											\
	{																													\
		Cosmos::Logger::GetInstance().ToTerminal(Cosmos::Logger::Severity::Assert, __FILE__, __LINE__, __VA_ARGS__);	\
		std::abort();																									\
	}																													\
}

#define VK_ASSERT(fn, ...)																								\
{																														\
	VkResult res = (fn);																								\
																														\
	if (res != VK_SUCCESS)																								\
	{																													\
		Cosmos::Logger::GetInstance().ToTerminal(Cosmos::Logger::Severity::Assert, __FILE__, __LINE__, __VA_ARGS__);	\
		std::abort();																									\
	}																													\
}

#else

#define LOG_ASSERT(x, ...)																								\
{																														\
	if(!(x))																											\
	{																													\
		Cosmos::Logger::GetInstance().ToTerminal(Cosmos::Logger::Severity::Assert, __FILE__, __LINE__, __VA_ARGS__);	\
		std::abort();																									\
	}																													\
}

#define VK_ASSERT(fn, ...)																								\
{																														\
	VkResult res = (fn);																								\
																														\
	if (res != VK_SUCCESS)																								\
	{																													\
		Cosmos::Logger::GetInstance().ToTerminal(Cosmos::Logger::Severity::Assert, __FILE__, __LINE__, __VA_ARGS__);	\
		std::abort();																									\
	}																													\
}

#endif