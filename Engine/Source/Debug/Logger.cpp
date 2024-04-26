#include "epch.h"
//#include "Logger.h"

namespace Cosmos
{
	Logger::~Logger()
	{
		printf("%s", mOutput.str().c_str());
	}

	Logger& Logger::Get()
	{
		static Logger instance;
		return instance;
	}

	void Logger::ToTerminal(Severity severity, const char* file, int line, const char* msg, ...)
	{
		char buffer[LOG_MAX_SIZE];
		va_list args;
		va_start(args, msg);
		vsnprintf(buffer, LOG_MAX_SIZE, msg, args);
		va_end(args);

		time_t now = time(NULL);
		char buf[40];
		struct tm tstruct = *localtime(&now);
		strftime(buf, sizeof(buf), "%X", &tstruct); // format: HH:MM:SS

		// external console
		{
			std::ostringstream oss;
			oss << "[" << buf << "]";
			oss << "[" << SeverityToConstChar(severity) << "]";
			oss << ": " << buffer;

			if (mConsoleMessages.size() >= LOG_MAX_ENTRIES_SIZE)
			{
				mConsoleMessages.clear();
			}

			oss << std::endl;

			mConsoleMessages.push_back({ severity, oss.str() });
		}

		// os console
		{
			std::ostringstream oss;
			oss << "[" << buf <<  "]";
			oss << "[" << file << " - " << line << "]";
			oss << "[" << SeverityToConstChar(severity) << "]";
			oss << ": " << buffer;

			printf("%s\n", oss.str().c_str());
		}
	}
	
	void Logger::ToFile(Severity severity, const char* path, const char* file, int line, const char* msg, ...)
	{
		FILE* f = std::fopen(path, "a+");

		char buffer[LOG_MAX_SIZE];
		va_list args;
		va_start(args, msg);
		vsnprintf(buffer, LOG_MAX_SIZE, msg, args);
		va_end(args);

		time_t now = time(NULL);
		char buf[40];
		struct tm tstruct = *localtime(&now);
		strftime(buf, sizeof(buf), "%X", &tstruct); // format: HH:MM:SS

		std::stringstream oss;
		oss << "[" << buf << "]";
		oss << "[" << file << " - " << line << "]";
		oss << "[" << SeverityToConstChar(severity) << "]";
		oss << ": " << buffer;

		fprintf(f, "%s\n", oss.str().c_str());

		fclose(f);
	}

	const char* Logger::SeverityToConstChar(Severity severity)
	{
		switch (severity)
		{
		case Severity::Trace: return "Trace";
		case Severity::Info: return "Info";
		case Severity::Todo: return "Todo";
		case Severity::Warn: return "Warning";
		case Severity::Error: return "Error";
		case Severity::Assert: return "Assertion";
		}

		return "Undefined Severity Level";
	}
}