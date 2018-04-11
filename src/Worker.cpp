#include "Worker.hpp"
#include "WorkerPool.hpp"
#include "Job.hpp"

/*
 * This file is part of CAM-RE and is distributed under the GPLv3 License.
 * See LICENSE for more details.
 *
 * (C) 2018 Hal Gentz
 */

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
