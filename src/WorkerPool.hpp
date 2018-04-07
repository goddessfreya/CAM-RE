#ifndef CAM_THREADPOOL_HPP
#define CAM_THREADPOOL_HPP

#include <vector>
#include <atomic>
#include <cstdint>
#include <mutex>

#include "CountedSharedMutex.hpp"
#include "Worker.hpp"
#include "ThreadSafeRandomNumberGenerator.tpp"

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
				i = 0;
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

	private:
	ThreadSafeRandomNumberGenerator<size_t> ranGen;
	int FindPullablePool();
	std::mutex pullJobMutex;
	CAM::CountedSharedMutex inFlightMutex;
	void WorkerRoutine();
	std::vector<std::unique_ptr<Worker>> workers;
};
}

#endif
