#include "Worker.hpp"
#include "WorkerPool.hpp"
#include "Job.hpp"

CAM::Worker::Worker(WorkerPool* owner, bool background)
	: owner(owner), background(background)
{
	static size_t count = 0;
	this->count = count;
	++count;
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
	owner->RegisterInFlightOperation();
	while (run)
	{
		if (!jobs.AnyRunnableJobs())
		{
			auto job = owner->TryPullingJob();

			if (job == nullptr)
			{
				owner->UnregisterInFlightOperation();
				while (job == nullptr)
				{
					if (!run || !background)
					{
						owner->UnregisterInFlightOperation();
						return;
					}
					std::this_thread::yield();

					owner->RegisterInFlightOperation();
					job = owner->TryPullingJob();

					if (job == nullptr)
					{
						owner->UnregisterInFlightOperation();
					}
				}
			}

			SubmitJob(std::move(job));
		}

		auto job = PullJob();
		if (job != nullptr)
		{
			job->DoJob();
		}
	}
	owner->UnregisterInFlightOperation();
}

bool CAM::Worker::JobPoolEmpty()
{
	return jobs.Empty();
}

bool CAM::Worker::JobPoolAnyRunnableJobs()
{
	return jobs.AnyRunnableJobs();
}

void CAM::Worker::RequestInactivity()
{
	run = false;
}
