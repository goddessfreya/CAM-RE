#include "WorkerPool.hpp"
#include "Job.hpp"

CAM::WorkerPool::~WorkerPool()
{
	// We cannot kill any Worker before any other Worker's thread dies else
	// shenanigans happen. So we wait for all of them to die first.
	for (auto& worker : workers)
	{
		worker->RequestInactivity();
	}

	while (inFlightOperations != 0)
	{
		std::this_thread::yield();
	}
}

void CAM::WorkerPool::AddWorker(std::unique_ptr<Worker> worker)
{
	workers.push_back(std::move(worker));
}

bool CAM::WorkerPool::SubmitJob(std::unique_ptr<Job> job)
{
	auto submitPool = ranGen(0, workers.size() - 1);
	++inFlightOperations;
	if (job == nullptr)
	{
		--inFlightOperations;
		return false;
	}

	if (submitPool >= workers.size())
	{
		submitPool = 0;
	}

	bool first = true;
	do
	{
		if (submitPool >= workers.size())
		{
			if (!first)
			{
				--inFlightOperations;
				return false;
			}
			submitPool = 0;
			first = false;
		}

		if (workers[submitPool] != nullptr)
		{
			workers[submitPool]->SubmitJob(std::move(job));
			++submitPool;
			--inFlightOperations;
			return true;
		}
		++submitPool;
	} while (true);
}

std::unique_ptr<CAM::Job> CAM::WorkerPool::TryPullingJob()
{
	int pullPool = FindPullablePool();
	++inFlightOperations;
	std::unique_lock<std::mutex> lock(pullJobMutex);
	if (workers[pullPool] != nullptr && workers[pullPool]->JobPoolAnyRunnableJobs())
	{
		auto ret = workers[pullPool]->PullJob();

		++pullPool;
		--inFlightOperations;
		return ret;
	}

	--inFlightOperations;
	return nullptr;
}

int CAM::WorkerPool::FindPullablePool()
{
	auto pullPool = ranGen(0, workers.size() - 1);
	bool first = true;
	do
	{
		if (pullPool >= workers.size())
		{
			if (!first)
			{
				return false;
			}
			pullPool = 0;
			first = false;
		}
		if (workers[pullPool] != nullptr && workers[pullPool]->JobPoolAnyRunnableJobs())
		{
			return true;
		}
		++pullPool;
	} while (true);

	return pullPool;
}

void CAM::WorkerPool::StartWorkers()
{
	for (auto& worker: workers)
	{
		worker->StartThread();
	}
}
