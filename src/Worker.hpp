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
class WorkerPool;
class Job;

class Worker
{
	public:
	Worker(WorkerPool* owner, bool background);
	~Worker(); // Jobs' jobs arn't returned to the thread pool because its dieing anyways.

	Worker(const Worker&) = delete;
	Worker(Worker&&) = default;
	Worker& operator=(const Worker&)& = delete;
	Worker& operator=(Worker&&)& = default;

	void StartThread();
	void SubmitJob(std::unique_ptr<Job> job);
	std::unique_ptr<Job> PullJob();

	bool JobPoolEmpty();
	bool JobPoolNoRunnableJobs();

	inline bool IsBackground() { return background; }

	void RequestInactivity();

	void WorkerRoutine();

	private:
	size_t count;

	WorkerPool* owner;
	bool run = true;
	std::unique_ptr<std::thread> thisThread;
	JobPool jobs;
	bool background;
};
}

#endif
