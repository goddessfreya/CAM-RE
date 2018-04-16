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

#include "Worker.hpp"
#include "WorkerPool.hpp"
#include "Job.hpp"

CAM::Worker::Worker(WorkerPool* owner, bool background)
	: owner(owner), background(background)
{
	static size_t lastThreadNumber = 0;
	threadNumber = lastThreadNumber;
	++lastThreadNumber;
}

CAM::Worker::~Worker()
{
	RequestInactivity(); // just in case
	if (background)
	{
		thisThread->join();
	}
}

void CAM::Worker::StartThread()
{
	if (!background)
	{
		return;
	}

	if (thisThread != nullptr)
	{
		RequestInactivity(); // just in case
		thisThread->join();
	}
	run = true;
	thisThread = std::make_unique<std::thread>(std::bind(&CAM::Worker::WorkerRoutine, this));
}

void CAM::Worker::SubmitJob(std::unique_ptr<Job> job)
{
	jobs.SubmitJob(std::move(job));
}

std::unique_ptr<CAM::Job> CAM::Worker::PullJob()
{
	return jobs.PullJob();
}

void CAM::Worker::WorkerRoutine()
{
	auto idleLock = owner->InFlightLock();
	std::unique_ptr<Job> retJob = nullptr;
	while (run)
	{
		if (retJob != nullptr)
		{
			auto newRetJob = retJob->DoJob(owner, threadNumber);
			owner->ReturnJob(std::move(retJob));
			std::swap(retJob, newRetJob);
			continue;
		}

		if (!jobs.NoRunnableJobs())
		{
			auto job = owner->TryPullingJob().first;

			if (job == nullptr)
			{
				idleLock = nullptr;
				while (job == nullptr)
				{
					if (!background && owner->GetInflightMutex().SharedCount() == 0 && !owner->NoJobs())
					{
						printf("%zu: Main left\n", threadNumber);
						return;
					}

					if (!run)
					{
						printf("%zu: Thread left\n", threadNumber);
						return;
					}
					std::this_thread::yield();

					auto ret = owner->TryPullingJob();

					if (ret.first != nullptr)
					{
						idleLock = std::move(ret.second);
						job = std::move(ret.first);
					}
				}
			}

			SubmitJob(std::move(job));
		}

		auto job = PullJob();
		if (job != nullptr)
		{
			retJob = job->DoJob(owner, threadNumber);
			owner->ReturnJob(std::move(job));
		}
	}
}

bool CAM::Worker::JobPoolEmpty()
{
	return jobs.Empty();
}

bool CAM::Worker::JobPoolNoRunnableJobs()
{
	return jobs.NoRunnableJobs();
}

void CAM::Worker::RequestInactivity()
{
	run = false;
}
