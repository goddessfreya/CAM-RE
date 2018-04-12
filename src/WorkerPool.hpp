#ifndef CAM_THREADPOOL_HPP
#define CAM_THREADPOOL_HPP

/*
 * This file is part of CAM-RE and is distributed under the GPLv3 License.
 * See LICENSE for more details.
 *
 * (C) 2018 Hal Gentz
 */

/*
 * This is the class responsible for workers, however, please note that it does
 * not initilize those workers. You must supply it with workers yourself,
 * perferably including one non-background worker for you to enter after
 * supplying the starting jobs.
 *
 * You can submit jobs though SubmitJob and you can pull jobs though PullJob,
 * however the submit will not work if you have no workers. The PullJob might
 * return nullptr if no job can be found.
 *
 * This class is also where you should get jobs from. DO NOT ALLOCATE THEM
 * YOURSELF, that would be awfully ineffiencent. Workers will return jobs to
 * this class when they use them. If you request a job and then do not submit it
 * then it is your responsibilty to return it.
 */

#include <vector>
#include <atomic>
#include <cstdint>
#include <mutex>

#include "Utils/CountedSharedMutex.hpp"
#include "Worker.hpp"
#include "Utils/ThreadSafeRandomNumberGenerator.tpp"
#include "Utils/Allocator.tpp"

namespace CAM
{
class Job;

class WorkerPool
{
	public:
	using JobLockPair = std::pair<std::unique_ptr<CAM::Job>, std::unique_ptr<std::shared_lock<CAM::CountedSharedMutex>>>;
	inline WorkerPool() {}
	~WorkerPool(); // Jobs' jobs arn't returned to the thread pool because its dieing anyways.

	void AddWorker(std::unique_ptr<Worker> worker);

	WorkerPool(const WorkerPool&) = delete;
	WorkerPool(WorkerPool&&) = default;
	WorkerPool& operator=(const WorkerPool&)& = delete;
	WorkerPool& operator=(WorkerPool&&)& = default;

	bool SubmitJob(std::unique_ptr<Job> job); // false for failure
	JobLockPair TryPullingJob();

	void StartWorkers();

	inline bool NoJobs()
	{
		size_t i = 0;
		while(i != workers.size())
		{
			if (!workers[i]->JobPoolEmpty() || inFlightMutex.SharedCount() != 0)
			{
				std::this_thread::yield();
				return false;
			}

			++i;
		}

		return true;
	}

	std::unique_ptr<std::shared_lock<CAM::CountedSharedMutex>> InFlightLock()
	{
		return std::make_unique<std::shared_lock<CAM::CountedSharedMutex>>(inFlightMutex);
	}

	inline CAM::CountedSharedMutex& GetInflightMutex()
	{
		return inFlightMutex;
	}

	template<typename... Args>
	inline std::unique_ptr<Job> GetJob(Args&&... args)
	{
		return jobAllocator(std::forward<Args>(args)...);
	}

	inline void ReturnJob(std::unique_ptr<Job> job)
	{
		jobAllocator.Return(std::move(job));
	}

	private:
	Allocator<Job> jobAllocator;
	ThreadSafeRandomNumberGenerator<size_t> ranGen;
	int FindPullablePool();
	std::mutex pullJobMutex;
	CAM::CountedSharedMutex inFlightMutex;
	void WorkerRoutine();
	std::vector<std::unique_ptr<Worker>> workers;
};
}

#endif
