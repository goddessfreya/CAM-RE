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
	// We cannot kill any Worker before any other Worker's thread dies else
	// shenanigans happen. So we wait for all of them to die first.
	for (auto& worker : workers)
	{
		worker->RequestInactivity();
	}

	std::unique_lock<std::shared_mutex> idleLock(inFlightMutex);
}

void CAM::Jobs::WorkerPool::AddWorker(std::unique_ptr<Worker> worker)
{
	workers.push_back(std::move(worker));
}

// TODO: Submit jobs Round-Robin-ly
bool CAM::Jobs::WorkerPool::SubmitJob(std::unique_ptr<Job> job)
{
	auto idleLock = InFlightLock();
	assert(job != nullptr);

	if (job->MainThreadOnly())
	{
		mainThreadJobs.SubmitJob(std::move(job));
		return true;
	}

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

	auto idleLock = InFlightLock();
	int pullPool = FindPullablePool();
	if (pullPool == -1)
	{
		return WorkerPool::JobLockPair(nullptr, nullptr);
	}

	std::unique_lock<std::mutex> lock(pullJobMutex);
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
	for (auto& worker: workers)
	{
		worker->StartThread();
	}
}
