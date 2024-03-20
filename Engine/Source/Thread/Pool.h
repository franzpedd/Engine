#pragma once

#include "AsyncResource.h"
#include "Util/Queue.h"

#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <thread>
#include <vector>

namespace Cosmos::thread
{
	class Pool
	{
	public:

		// constructor
		Pool(const int threads);

		// destructor
		~Pool();

		// delete copy constructor
		Pool(const Pool&) = delete;

		// delete move constructor
		Pool(Pool&&) = delete;

		// delete asignment constructor
		Pool& operator=(const Pool&) = delete;

		// delete move assignment constructor
		Pool& operator=(Pool&&) = delete;

	public:

		template<typename F, typename...Args>
		auto Enqueue(F&& f, Args&&... args) -> std::future<decltype(f(args...))>
		{
			// create a function with bounded parameters ready to execute
			std::function<decltype(f(args...))()> func = std::bind(std::forward<F>(f), std::forward<Args>(args)...);

			// encapsulate it into a shared ptr in order to be able to copy construct / assign 
			auto task_ptr = std::make_shared<std::packaged_task<decltype(f(args...))()>>(func);

			// wrap packaged task into void function
			std::function<void()> wrapper_func = [task_ptr]() { (*task_ptr)(); };

			// enqueue generic wrapper function
			mQueue.enqueue(wrapper_func);

			// wake up one thread if its waiting
			mCondition.notify_one();

			// return future from promise
			return task_ptr->get_future();
		}

	public: // temp

		bool mShutdown = false;
		util::Queue<std::function<void()>> mQueue;
		std::vector<std::thread> mThreads;
		std::mutex mMutex;
		std::condition_variable mCondition;
	};

	class PoolManager
	{
	public:

		// returns the pool manager singleton
		static PoolManager& GetInstance();

		// delete copy constructor
		PoolManager(const PoolManager&) = delete;

		// delete assignment constructor
		PoolManager& operator=(const PoolManager&) = delete;

	public:

		// returns a reference for the resources pool, used to load resources in other threads
		inline std::unique_ptr<Pool>& GetResourcesPool() { return mResourcesPool; }

	private:

		// constructor
		PoolManager();

	private:

		static PoolManager* sInstance;
		std::unique_ptr<Pool> mResourcesPool;
	};

	class Worker
	{
	public:

		// constructor
		Worker(Pool* pool, const int id);

		// overload operator ()
		void operator()();

	private:

		int mID;
		Pool* mPool;
	};
}