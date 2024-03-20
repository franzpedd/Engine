#pragma once

#include <mutex>
#include <queue>

namespace Cosmos::util
{
    // thread safe implementation of a queue using an std::queue
    template <typename T>
    class Queue
    {
    public:

        // constructor
        Queue() = default;

        // destructor
        ~Queue() = default;

        // returns if queue is empty
        bool empty()
        {
            std::unique_lock<std::mutex> lock(mMutex);
            return mQueue.empty();
        }

        // returns the queue size
        int size()
        {
            std::unique_lock<std::mutex> lock(mMutex);
            return mQueue.size();
        }

        // pushes into the queue
        void enqueue(T& t)
        {
            std::unique_lock<std::mutex> lock(mMutex);
            mQueue.push(t);
        }

        // pops out of the queue
        bool dequeue(T& t)
        {
            std::unique_lock<std::mutex> lock(mMutex);

            if (mQueue.empty())
              return false;

            t = std::move(mQueue.front());

            mQueue.pop();
            return true;
        }

    private:
        std::queue<T> mQueue;
        std::mutex mMutex;
    };
}