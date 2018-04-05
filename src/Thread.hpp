#ifndef CAM_THREAD_HPP
#define CAM_THREAD_HPP

#include <vector>
#include <atomic>
#include <thread>
#include <cstdint>
#include <shared_mutex>
#include <mutex>

#include "JobPool.hpp"

namespace CAM
{
class ThreadPool;
class Job;

class Thread
{
	public:
	Thread(ThreadPool* owner);
	~Thread(); // Jobs' jobs arn't returned to the thread pool because its dieing anyways.

	Thread(const Thread&) = delete;
	Thread(Thread&&) = default;
	Thread& operator=(const Thread&)& = delete;
	Thread& operator=(Thread&&)& = default;

	void Start();
	void SubmitJob(std::unique_ptr<Job> job);
	std::unique_ptr<Job> PullJob();

	bool JobPoolEmpty();

	void RequestInactivity();

	private:
	uint32_t pullPool = 0;
	size_t count;
	void ThreadRoutine();

	ThreadPool* owner;
	bool run = true;
	std::unique_ptr<std::thread> thisThread;
	JobPool jobs;
	std::atomic<uint32_t> users = 0;
};
}

#endif
