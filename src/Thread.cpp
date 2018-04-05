#include "Thread.hpp"
#include "ThreadPool.hpp"
#include "Job.hpp"

CAM::Thread::Thread(ThreadPool* owner)
	: owner(owner)
{
	static size_t count = 0;
	this->count = count;
	++count;

	Start();
}

CAM::Thread::~Thread()
{
	RequestInactivity(); // just in case
	thisThread->join();
}

void CAM::Thread::Start()
{
	if (thisThread != nullptr)
	{
		RequestInactivity(); // just in case
		thisThread->join();
	}
	run = true;
	thisThread = std::make_unique<std::thread>(std::bind(&CAM::Thread::ThreadRoutine, this));
}

void CAM::Thread::SubmitJob(std::unique_ptr<Job> job)
{
	jobs.SubmitJob(std::move(job));
}

std::unique_ptr<CAM::Job> CAM::Thread::PullJob()
{
	return jobs.PullJob();
}

void CAM::Thread::ThreadRoutine()
{
	auto unIdleLock = owner->UnIdleLock();
	while (run)
	{
		if (jobs.empty())
		{
			auto job = owner->PullJob(pullPool);

			if (job == nullptr)
			{
				unIdleLock = nullptr;
				while (job == nullptr)
				{
					if (!run)
					{
						owner->RegisterInactvitiy();
						return;
					}
					std::this_thread::yield();
					job = owner->PullJob(pullPool);
				}
				unIdleLock = owner->UnIdleLock();
			}
			SubmitJob(std::move(job));
		}

		auto job = PullJob();
		if (job != nullptr)
		{
			job->DoJob();
		}
	}
	owner->RegisterInactvitiy();
}

bool CAM::Thread::JobPoolEmpty()
{
	return jobs.empty();
}

void CAM::Thread::RequestInactivity()
{
	run = false;
}
