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
 * This is a (hopefully) thread-safe class which stores jobs. (LIFO)
 *
 * Each worker owns its own pool, which it draws jobs from until empty.
 */

#ifndef CAM_JOBS_JOBPOOL_HPP
#define CAM_JOBS_JOBPOOL_HPP

#include <vector>
#include <mutex>
#include <shared_mutex>
#include <thread>
#include <algorithm>
#include <cstdint>

#include "../Utils/Aligner.tpp"

namespace CAM
{
namespace Jobs
{
class Job;
class WorkerPool;

// TODO: If its too slow, try to use atomics instead
// https://manu343726.github.io/2017/03/13/lock-free-job-stealing-task-system-with-modern-c.html
class JobPool
{
	public:
	JobPool(WorkerPool* wp);

	void SubmitJob(std::unique_ptr<Job> job);
	std::unique_ptr<Job> PullJob();

	[[nodiscard]] bool Empty();
	[[nodiscard]] bool NoRunnableJobs();

	[[nodiscard]] size_t RunnableJobsLeft();

	void MakeRunnable(Job* job);

	[[nodiscard]] std::unique_ptr<Job> PullDepJob(Job* job);

	private:
	std::shared_mutex jobsMutex;
	std::shared_mutex jobsWithUnmetDepsMutex;
	std::vector<std::unique_ptr<Job>> jobs;
	std::vector<std::unique_ptr<Job>> jobsWithUnmetDeps;
	WorkerPool* wp;
};
}
}

#endif
