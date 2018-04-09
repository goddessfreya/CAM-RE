#ifndef CAM_JOBPOOL_HPP
#define CAM_JOBPOOL_HPP

/*
 * This file is part of CAM-RE and is distributed under the GPLv3 License.
 * See LICENSE for more details.
 *
 * (C) 2018 Hal Gentz
 */

/*
 * This is a (hopefully) thread-safe class which stores jobs. (LIFO)
 *
 * Each worker owns its own pool, which it draws jobs from until empty.
 */

#include <vector>
#include <mutex>
#include <shared_mutex>
#include <thread>
#include <algorithm>
#include <cstdint>

#include "Aligner.tpp"

namespace CAM
{
class Job;

class JobPool
{
	public:
	void SubmitJob(std::unique_ptr<Job> job);
	std::unique_ptr<Job> PullJob();

	bool Empty();
	bool NoRunnableJobs();

	void MakeRunnable(Job* job);

	std::unique_ptr<Job> PullDepJob(Job* job);

	private:
	std::shared_mutex jobsMutex;
	std::shared_mutex jobsWithUnmetDepsMutex;
	std::vector<std::unique_ptr<Job>> jobs;
	std::vector<std::unique_ptr<Job>> jobsWithUnmetDeps;
};
}

#endif
