#ifndef CAM_THREAD_HPP
#define CAM_THREAD_HPP

/*
 * This file is part of CAM-RE and is distributed under the GPLv3 License.
 * See LICENSE for more details.
 *
 * (C) 2018 Hal Gentz
 */

/*
 * This is a Worker who will do work. Background workers spawn their own
 * thread, non-backgrounds don't.
 *
 * Each worker owns its own pool, which it draws jobs from until empty. It will
 * then try to draw jobs from other job pools, maybe butchering performace.
 */

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
	size_t threadNumber;

	WorkerPool* owner;
	bool run = true;
	std::unique_ptr<std::thread> thisThread;
	JobPool jobs;
	bool background;
};
}

#endif
