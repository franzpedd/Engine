#pragma once

#include <mutex>
#include <fstream>
#include <thread>

namespace Cosmos
{
	class Profiler
	{
	public:

		struct Result
		{
			const char* name;
			std::chrono::duration<double, std::micro> start;
			std::chrono::microseconds time;
			std::thread::id thread;
		};

		struct Timer
		{
			const char* name;
			bool stopped;
			std::chrono::time_point<std::chrono::steady_clock> start;

			// constructor
			Timer(const char* name);

			// destructor
			~Timer();

			// stops the timer
			void Stop();
		};

	public:

		// constructor
		Profiler() = default;

		// destructor
		~Profiler() = default;

		// returns a reference to the profiler
		static Profiler& Get();

	public:

		// initializes a session
		void Begin(const char* name, const char* filePath = "profiler.json");

		// ends a session
		void End();

	private:

		// ends the current session (internaly handled so user only needs to care with the other func)
		void InternalEnd();

		// writes the begining of the profiler json
		void WriteHeader();

		// writes the scope results on profiler json
		void Write(const Result& result);

		// writes the ending of the profiler json
		void WriteFooter();

	private:

		std::mutex mMutex;
		bool mInUse = false;
		std::ofstream mOutputStream;
	};
}

#define ENGINE_PROFILER 1
#if defined(ENGINE_PROFILER)
	#if defined (_MSC_VER)
		#define FUNC_SIG __FUNCSIG__
	#elif
		#define FUNC_SIG __func__
	#else
		#define FUNC_SIG "UNKNOWN FUNC SIGNATURE"
	#endif

	#define PROFILER_BEGIN(name, filepath) Cosmos::Profiler::Get().Begin(name, filepath)
	#define PROFILER_END() Cosmos::Profiler::Get().End()
	#define PROFILER_SCOPE(name) Cosmos::Profiler::Timer timer##__LINE__(name);
	#define PROFILER_FUNCTION() PROFILER_SCOPE(FUNC_SIG)
#elif
	#define PROFILER_BEGIN_SESSION(name, filepath)
	#define PROFILER_END_SESSION()
	#define PROFILER_SCOPE(name)
	#define PROFILER_FUNCTION()
#endif