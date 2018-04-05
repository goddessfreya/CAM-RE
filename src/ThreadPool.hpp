#ifndef CAM_THREADPOOL_HPP
#define CAM_THREADPOOL_HPP

#include <vector>
#include <atomic>
#include <cstdint>
#include <mutex>
#include <shared_mutex>

#include "Thread.hpp"

namespace CAM
{
class Job;

class ThreadPool
{
	public:
	ThreadPool(uint32_t numberOfThreads);
	~ThreadPool(); // Jobs' jobs arn't returned to the thread pool because its dieing anyways.

	ThreadPool(const ThreadPool&) = delete;
	ThreadPool(ThreadPool&&) = default;
	ThreadPool& operator=(const ThreadPool&)& = delete;
	ThreadPool& operator=(ThreadPool&&)& = default;

	bool SubmitJob(std::unique_ptr<Job> job, uint32_t& submitPool); // false for failure
	std::unique_ptr<Job> PullJob(uint32_t& pullPool);

	inline std::unique_ptr<std::shared_lock<std::shared_mutex>> UnIdleLock()
	{
		return std::make_unique<std::shared_lock<std::shared_mutex>>(idleMutex);
	}

	inline void WaitTillIdle()
	{
		std::this_thread::yield();

		std::unique_lock<std::shared_mutex> lock(idleMutex);
	}

	inline void WaitTillNoJobs()
	{
		std::this_thread::yield();

		uint32_t i = 0;
		while(i != threads.size())
		{
			if (!threads[i]->JobPoolEmpty())
			{
				i = 0;
				std::this_thread::yield();
				continue;
			}

			i++;
		}
	}

	void RegisterInactvitiy();

	private:
	std::atomic<uint32_t> runningThreads;
	std::shared_mutex idleMutex;
	std::vector<std::unique_ptr<Thread>> threads;
};
}

#endif
