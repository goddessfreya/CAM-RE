/*
 * Copyright (C) 2018 Hal Gentz
 *
 * This file is part of CAM-RE.
 *
 * CAM-RE is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * Bash is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * CAM-RE. If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * This is a Worker who will do work. Background workers spawn their own
 * thread, non-backgrounds don't.
 *
 * Each worker owns its own pool, which it draws jobs from until empty. It will
 * then try to draw jobs from other job pools, maybe butchering performance.
 */

#ifndef CAM_JOBS_THREAD_HPP
#define CAM_JOBS_THREAD_HPP

#include <vector>
#include <atomic>
#include <thread>
#include <cstdint>
#include <shared_mutex>
#include <mutex>

#include "JobPool.hpp"

namespace CAM
{
namespace Jobs
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
	[[nodiscard]] std::unique_ptr<Job> PullJob();

	[[nodiscard]] bool JobPoolEmpty();
	[[nodiscard]] bool JobPoolNoRunnableJobs();

	[[nodiscard]] inline bool IsBackground() const { return background; }

	void RequestInactivity();

	void WorkerRoutine();

	private:
	size_t threadNumber;

	WorkerPool* owner;
	std::atomic<bool> run = true;
	std::unique_ptr<std::thread> thisThread;
	JobPool jobs;
	bool background;
};
}
}

#endif
