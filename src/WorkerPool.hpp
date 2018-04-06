#ifndef CAM_THREADPOOL_HPP
#define CAM_THREADPOOL_HPP

#include <vector>
#include <atomic>
#include <cstdint>
#include <mutex>
#include <shared_mutex>

#include "Worker.hpp"
#include "ThreadSafeRandomNumberGenerator.tpp"

namespace CAM
{
class Job;

class WorkerPool
{
	public:
	inline WorkerPool() {}
	~WorkerPool(); // Jobs' jobs arn't returned to the thread pool because its dieing anyways.

	void AddWorker(std::unique_ptr<Worker> worker);

	WorkerPool(const WorkerPool&) = delete;
	WorkerPool(WorkerPool&&) = default;
	WorkerPool& operator=(const WorkerPool&)& = delete;
	WorkerPool& operator=(WorkerPool&&)& = default;

	bool SubmitJob(std::unique_ptr<Job> job); // false for failure
	std::unique_ptr<Job> TryPullingJob();

	void StartWorkers();

	inline void WaitTillNoJobs()
	{
		printf("waiting for no jobs\n");
		std::this_thread::yield();

		size_t i = 0;
		while(i != workers.size())
		{
			if (!workers[i]->JobPoolEmpty() || inFlightOperations > 0)
			{
				i = 0;
				std::this_thread::yield();
				continue;
			}

			++i;
		}
		printf("finished waiting for no jobs\n");
	}

	void RegisterInFlightOperation()
	{
		++inFlightOperations;
	}

	void UnregisterInFlightOperation()
	{
		--inFlightOperations;
	}

	private:
	ThreadSafeRandomNumberGenerator<size_t> ranGen;
	int FindPullablePool();
	std::mutex pullJobMutex;
	std::atomic<int> inFlightOperations = 0;
	void WorkerRoutine();
	std::vector<std::unique_ptr<Worker>> workers;
};
}

#endif
