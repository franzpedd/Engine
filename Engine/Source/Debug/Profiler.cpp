#include "epch.h"
//#include "Profiler.h"

namespace Cosmos
{
	Profiler& Profiler::Get()
	{
		static Profiler instance;
		return instance;
	}

	void Profiler::Begin(const char* name, const char* filePath)
	{
		// create profiler folder
		if (!std::filesystem::is_directory(std::filesystem::current_path() /+ "Profiler"))
		{
			std::filesystem::create_directory(std::filesystem::current_path() /+ "Profiler");
		}

		// append profiler folder at the begining
		std::filesystem::path path = std::filesystem::current_path();
		path /= "Profiler";
		path /= filePath;

		std::lock_guard lock(mMutex);

		if (mInUse)
		{
			LOG_TO_TERMINAL(Logger::Severity::Warn, "Profiler is already in use, clearing previous use and initializing new one");
			InternalEnd();
		}

		mInUse = true;
		mOutputStream.open(path.c_str());
		WriteHeader();
	}

	void Profiler::End()
	{
		std::lock_guard lock(mMutex);
		InternalEnd();
	}

	void Profiler::InternalEnd()
	{
		if (mInUse)
		{
			WriteFooter();
			mOutputStream.close();
			mInUse = false;
		}
	}

	void Profiler::WriteHeader()
	{
		mOutputStream << "{\"otherData\": {},\"traceEvents\":[{}";
		mOutputStream.flush();
	}

	void Profiler::Write(const Result& result)
	{
		std::lock_guard lock(mMutex);

		std::stringstream json{};
		json << std::setprecision(3) << std::fixed;
		json << ",{";
		json << "\"cat\":\"function\",";
		json << "\"dur\":" << (result.time.count()) << ',';
		json << "\"name\":\"" << result.name << "\",";
		json << "\"ph\":\"X\",";
		json << "\"pid\":0,";
		json << "\"tid\":" << result.thread << ",";
		json << "\"ts\":" << result.start.count();
		json << "}";
		
		if(mInUse)
		{
			mOutputStream << json.str();
			mOutputStream.flush();
		}
	}

	void Profiler::WriteFooter()
	{
		mOutputStream << "]}";
		mOutputStream.flush();
	}

	Profiler::Timer::Timer(const char* name)
		: name(name), stopped(false)
	{
		start = std::chrono::steady_clock::now();
	}

	Profiler::Timer::~Timer()
	{
		if (!stopped)
		{
			Stop();
		}
	}

	void Profiler::Timer::Stop()
	{
		auto end = std::chrono::steady_clock::now();
		auto hrStart = std::chrono::duration<double, std::micro>{ start.time_since_epoch() };
		auto elapsed = std::chrono::time_point_cast<std::chrono::microseconds>(end).time_since_epoch() - std::chrono::time_point_cast<std::chrono::microseconds>(start).time_since_epoch();
	
		Profiler::Get().Write({ name , hrStart, elapsed, std::this_thread::get_id() });
		stopped = true;
	}
}