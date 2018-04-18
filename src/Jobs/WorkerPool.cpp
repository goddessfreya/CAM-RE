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

#include "WorkerPool.hpp"
#include "Job.hpp"

#include <cassert>

CAM::Jobs::WorkerPool::~WorkerPool()
{
	shutingDown = true;
	// We cannot kill any Worker before any other Worker's thread dies else
	// shenanigans happen. So we wait for all of them to die first.
	{
		auto lock = WorkersLock();
		while (!lock) { std::this_thread::yield(); lock = WorkersLock(); }
		for (auto& worker : workers)
		{
			worker->RequestInactivity();
		}
	}

	std::unique_lock<CAM::Utils::CountedSharedMutex> idleLock(inFlightMutex, std::defer_lock);
	std::unique_lock<std::shared_mutex> lock(workersMutex, std::defer_lock);
	std::lock(lock, idleLock);

	for (auto& worker : workers)
	{
		worker = nullptr;
	}
}

void CAM::Jobs::WorkerPool::AddWorker(std::unique_ptr<Worker> worker)
{
	std::unique_lock<std::shared_mutex> lock(workersMutex);
	workers.push_back(std::move(worker));
}

// TODO: Submit jobs Round-Robin-ly
bool CAM::Jobs::WorkerPool::SubmitJob(std::unique_ptr<Job> job)
{
	if (shutingDown)
	{
		return true; // Wasn't actually submited, but they don't need to know that
	}

	auto idleLock = InFlightLock();
	assert(job != nullptr);

	if (job->MainThreadOnly())
	{
		mainThreadJobs.SubmitJob(std::move(job));
		return true;
	}

	auto lock = WorkersLock();
	auto submitPool = ranGen(0, workers.size() - 1);

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

CAM::Jobs::WorkerPool::JobLockPair CAM::Jobs::WorkerPool::TryPullingJob()
{

	if (shutingDown)
	{
		return WorkerPool::JobLockPair(nullptr, nullptr);
	}

	int pullPool = FindPullablePool();
	if (pullPool == -1)
	{
		return WorkerPool::JobLockPair(nullptr, nullptr);
	}
	auto idleLock = InFlightLock();
	if (idleLock == nullptr)
	{
		return WorkerPool::JobLockPair(nullptr, nullptr);
	}

	while (true)
	{
		if (shutingDown)
		{
			return WorkerPool::JobLockPair(nullptr, nullptr);
		}
		std::unique_lock<std::mutex> lock1(pullJobMutex, std::defer_lock);
		std::shared_lock<std::shared_mutex> lock2(workersMutex, std::defer_lock);
		if (std::try_lock(lock1, lock2))
		{
			break;
		}
	}

	if (workers[pullPool] != nullptr && !workers[pullPool]->JobPoolNoRunnableJobs())
	{
		auto ret = workers[pullPool]->PullJob();

		++pullPool;
		return WorkerPool::JobLockPair(std::move(ret), std::move(idleLock));
	}

	return WorkerPool::JobLockPair(nullptr, nullptr);
}

int CAM::Jobs::WorkerPool::FindPullablePool()
{
	if (shutingDown) { return -1; }
	auto lock = WorkersLock();
	auto pullPool = ranGen(0, workers.size() - 1);
	bool first = true;
	while (true)
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
		if (workers[pullPool] != nullptr && !workers[pullPool]->JobPoolNoRunnableJobs())
		{
			return pullPool;
		}
		++pullPool;
	}
}

void CAM::Jobs::WorkerPool::StartWorkers()
{
	if (shutingDown) { return; }
	auto lock = WorkersLock();
	for (auto& worker: workers)
	{
		worker->StartThread();
	}
}
