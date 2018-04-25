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
 * This is the class responsible for workers, however, please note that it does
 * not initialize those workers. You must supply it with workers yourself,
 * preferably including one non-background worker for you to enter after
 * supplying the starting jobs.
 *
 * You can submit jobs though SubmitJob and you can pull jobs though PullJob,
 * however the submit will not work if you have no workers. The PullJob might
 * return nullptr if no job can be found.
 *
 * This class is also where you should get jobs from. DO NOT ALLOCATE THEM
 * YOURSELF, that would be awfully inefficient. Workers will return jobs to
 * this class when they use them. If you request a job and then do not submit it
 * then it is your responsibility to return it.
 */

#ifndef CAM_JOBS_THREADPOOL_HPP
#define CAM_JOBS_THREADPOOL_HPP

#include <vector>
#include <atomic>
#include <cstdint>
#include <mutex>

#include "../Utils/CountedSharedMutex.hpp"
#include "Worker.hpp"
#include "../Utils/ThreadSafeRandomNumberGenerator.tpp"
#include "../Utils/Allocator.tpp"

namespace CAM
{
namespace Jobs
{
class Job;

class WorkerPool
{
	public:
	using JobLockPair = std::pair<std::unique_ptr<Job>, std::unique_ptr<std::shared_lock<CAM::Utils::CountedSharedMutex>>>;
	WorkerPool() : mainThreadJobs(this) {}
	~WorkerPool(); // Jobs' jobs arn't returned to the thread pool because its dieing anyways.

	void AddWorker(std::unique_ptr<Worker> worker);

	WorkerPool(const WorkerPool&) = default;
	WorkerPool(WorkerPool&&) = delete;
	WorkerPool& operator=(const WorkerPool&)& = default;
	WorkerPool& operator=(WorkerPool&&)& = delete;

	[[nodiscard]] bool SubmitJob(std::unique_ptr<Job> job); // false for failure
	[[nodiscard]] JobLockPair TryPullingJob(bool background);

	void StartWorkers();

	[[nodiscard]] inline bool NoJobs() const
	{
		size_t i = 0;
		auto lock = WorkersLock();
		while(i != workers.size())
		{
			if (!lock) { return false; }

			if
			(
				inFlightMutex.SharedLockersLeft() != 0
				|| !workers[i]->JobPoolNoRunnableJobs()
				|| !mainThreadJobs.NoRunnableJobs())
			{
				return false;
			}

			++i;
		}

		return true;
	}

	[[nodiscard]] std::unique_ptr<std::shared_lock<std::shared_mutex>> WorkersLock() const
	{
		auto ret = std::make_unique<std::shared_lock<std::shared_mutex>>(workersMutex, std::defer_lock);
		if (ret->try_lock())
		{
			return ret;
		}

		return nullptr;
	}

	[[nodiscard]] std::unique_ptr<std::shared_lock<CAM::Utils::CountedSharedMutex>> InFlightLock() const
	{
		if (shutingDown.load(std::memory_order_acquire))
		{
			return nullptr;
		}

		auto ret = std::make_unique<std::shared_lock<CAM::Utils::CountedSharedMutex>>(inFlightMutex, std::defer_lock);
		if (ret->try_lock())
		{
			return ret;
		}

		return nullptr;
	}

	[[nodiscard]] inline CAM::Utils::CountedSharedMutex& GetInflightMutex() const
	{
		return inFlightMutex;
	}

	template<typename... Args>
	[[nodiscard]] inline std::unique_ptr<Job> GetJob(Args&&... args)
	{
		return jobAllocator(std::forward<Args>(args)...);
	}

	inline void ReturnJob(std::unique_ptr<Job> job)
	{
		jobAllocator.Return(std::move(job));
	}

	[[nodiscard]] inline JobPool& MainThreadJobs()
	{
		return mainThreadJobs;
	}

	void WakeUpThreads(size_t number);

	private:
	int FindPullablePool() const;

	Utils::Allocator<Job> jobAllocator;
	mutable Utils::ThreadSafeRandomNumberGenerator<size_t> ranGen;
	mutable std::mutex pullJobMutex;
	mutable CAM::Utils::CountedSharedMutex inFlightMutex;
	mutable std::shared_mutex workersMutex;
	std::vector<std::unique_ptr<Worker>> workers;

	JobPool mainThreadJobs;

	std::atomic<bool> shutingDown = false;
};
}
}

#endif
