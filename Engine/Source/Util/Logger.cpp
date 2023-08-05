#include "Logger.h"

#include <ctime>
#include <cstdarg>

namespace Cosmos
{
	Logger::~Logger()
	{
		printf("%s", m_Output.str().c_str());
	}
	
	void Logger::ToTerminal(Severity severity, const char* file, int line, const char* msg, ...)
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
	
	void Logger::ToFile(Severity severity, const char* path, const char* file, int line, const char* msg, ...)
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
	
	const char* Logger::SeverityToConstChar(Severity severity)
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
}