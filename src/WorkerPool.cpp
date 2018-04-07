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

	std::unique_lock<std::shared_mutex> idleLock(inFlightMutex);
}

void CAM::WorkerPool::AddWorker(std::unique_ptr<Worker> worker)
{
	workers.push_back(std::move(worker));
}

bool CAM::WorkerPool::SubmitJob(std::unique_ptr<Job> job)
{
	auto submitPool = ranGen(0, workers.size() - 1);
	auto idleLock = InFlightLock();
	if (job == nullptr)
	{
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
				return false;
			}
			submitPool = 0;
			first = false;
		}

		if (workers[submitPool] != nullptr)
		{
			workers[submitPool]->SubmitJob(std::move(job));
			++submitPool;
			return true;
		}
		++submitPool;
	} while (true);
}

CAM::WorkerPool::JobLockPair CAM::WorkerPool::TryPullingJob()
{

	auto idleLock = InFlightLock();
	int pullPool = FindPullablePool();
	if (pullPool == -1)
	{
		return WorkerPool::JobLockPair(nullptr, nullptr);
	}

	std::unique_lock<std::mutex> lock(pullJobMutex);
	if (workers[pullPool] != nullptr && workers[pullPool]->JobPoolAnyRunnableJobs())
	{
		auto ret = workers[pullPool]->PullJob();

		++pullPool;
		return WorkerPool::JobLockPair(std::move(ret), std::move(idleLock));
	}

	return WorkerPool::JobLockPair(nullptr, nullptr);
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
				return -1;
			}
			pullPool = 0;
			first = false;
		}
		if (workers[pullPool] != nullptr && workers[pullPool]->JobPoolAnyRunnableJobs())
		{
			return pullPool;
		}
		++pullPool;
	} while (true);
}

void CAM::WorkerPool::StartWorkers()
{
	for (auto& worker: workers)
	{
		worker->StartThread();
	}
}
