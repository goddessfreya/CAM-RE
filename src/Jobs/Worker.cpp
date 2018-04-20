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

CAM::Jobs::Worker::Worker(WorkerPool* owner, bool background)
	: owner(owner), background(background)
{
	static size_t lastThreadNumber = 0;
	threadNumber = lastThreadNumber;
	++lastThreadNumber;
}

CAM::Jobs::Worker::~Worker()
{
	RequestInactivity(); // just in case
	if (background)
	{
		thisThread->join();
	}
}

void CAM::Jobs::Worker::StartThread()
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
	thisThread = std::make_unique<std::thread>(std::bind(&CAM::Jobs::Worker::WorkerRoutine, this));
}

void CAM::Jobs::Worker::SubmitJob(std::unique_ptr<Job> job)
{
	size_t left;
	if ((left = jobs.RunnableJobsLeft()) != 0)
	{
		owner->WakeUpThreads(left);
	}

	jobs.SubmitJob(std::move(job));
}

std::unique_ptr<CAM::Jobs::Job> CAM::Jobs::Worker::PullJob()
{
	return jobs.PullJob();
}

void CAM::Jobs::Worker::WorkerRoutine()
{
	auto idleLock = owner->InFlightLock();
	if (!run || idleLock == nullptr)
	{
		printf("%zu: Thread left\n", threadNumber);
		return;
	}
	std::unique_ptr<Job> retJob = nullptr;
	while (run)
	{
		if (retJob != nullptr)
		{
			if (background && retJob->MainThreadOnly())
			{
				if (!owner->SubmitJob(std::move(retJob))) { throw std::runtime_error("Could not submit job\n"); }
			}
			else
			{
				auto newRetJob = retJob->DoJob(owner, threadNumber);
				owner->ReturnJob(std::move(retJob));
				std::swap(retJob, newRetJob);
				continue;
			}
		}

		if (!background && !owner->MainThreadJobs().NoRunnableJobs())
		{
			auto job = owner->MainThreadJobs().PullJob();
			retJob = job->DoJob(owner, threadNumber);
			owner->ReturnJob(std::move(job));
			continue;
		}

		if (jobs.NoRunnableJobs())
		{
			retJob = owner->TryPullingJob(background).first;

			if (retJob == nullptr)
			{
				idleLock = nullptr;
				owner->WakeUpMain();
				while (retJob == nullptr)
				{
					idleLock = nullptr;
					if (!background && owner->GetInflightMutex().SharedCount() == 0 && owner->NoJobs())
					{
						printf("%zu: Main left\n", threadNumber);
						return;
					}

					if (!run)
					{
						printf("%zu: Thread left\n", threadNumber);
						return;
					}

					if (!background && !owner->MainThreadJobs().NoRunnableJobs())
					{
						idleLock = owner->InFlightLock();
						retJob = owner->MainThreadJobs().PullJob();
						continue;
					}

					if (!jobs.NoRunnableJobs())
					{

						idleLock = owner->InFlightLock();
						retJob = PullJob();
						continue;
					}

					auto ret = owner->TryPullingJob(background);

					if (ret.first != nullptr && ret.second != nullptr)
					{
						idleLock = std::move(ret.second);
						retJob = std::move(ret.first);
						continue;
					}

					//if (background)
					{
						cc.Wait();
					}
				}
			}
			continue;
		}

		auto job = PullJob();
		if (!run)
		{
			printf("%zu: Thread left\n", threadNumber);
			return;
		}
		if (job != nullptr)
		{
			retJob = job->DoJob(owner, threadNumber);
			owner->ReturnJob(std::move(job));
		}
	}
	printf("%zu: Thread left\n", threadNumber);
	return;
}

bool CAM::Jobs::Worker::JobPoolEmpty()
{
	return jobs.Empty();
}

bool CAM::Jobs::Worker::JobPoolNoRunnableJobs()
{
	return jobs.NoRunnableJobs();
}

void CAM::Jobs::Worker::RequestInactivity()
{
	run = false;
	cc.Reset();
}

void CAM::Jobs::Worker::WakeUp()
{
	cc.Signal();
}
