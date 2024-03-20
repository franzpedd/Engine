#include "epch.h"
#include "Pool.h"

#include "Defines.h"

namespace Cosmos::thread
{
    PoolManager* PoolManager::sInstance = nullptr;

    Pool::Pool(const int threads)
    {
        mThreads.resize(threads);

        for (int i = 0; i < mThreads.size(); ++i)
        {
            mThreads[i] = std::thread(Worker(this, i));
        }
    }

    Pool::~Pool()
    {
        mShutdown = true;
        mCondition.notify_all();

        for (int i = 0; i < mThreads.size(); ++i)
        {
            if (mThreads[i].joinable())
            {
                mThreads[i].join();
            }
        }
    }

	PoolManager& PoolManager::GetInstance()
	{
        if (sInstance == nullptr)
        {
            if (sInstance == nullptr)
            {
                sInstance = new PoolManager();
            }
        }
        return *sInstance;
	}

    PoolManager::PoolManager()
    {
        mResourcesPool = std::make_unique<Pool>(RESOURCES_THREAD_SOUND_COUNT);
    }
   
    Worker::Worker(Pool* pool, const int id)
        : mPool(pool), mID(id)
    {
    }

    void Worker::operator()()
    {
        std::function<void()> func;
        bool dequeued;

        while (!mPool->mShutdown)
        {
            {
                std::unique_lock<std::mutex> lock(mPool->mMutex);

                if (mPool->mQueue.empty())
                    mPool->mCondition.wait(lock);

                dequeued = mPool->mQueue.dequeue(func);
            }

            if (dequeued)
                func();
        }
    }
}