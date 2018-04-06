#include "JobPool.hpp"
#include "Job.hpp"

void CAM::JobPool::SubmitJob(std::unique_ptr<Job> job)
{
	std::unique_lock<std::shared_mutex> lock(sMutex);
	jobs.push_back(std::move(job));
}

std::unique_ptr<CAM::Job> CAM::JobPool::PullJob()
{
	std::unique_lock<std::shared_mutex> lock(sMutex);
	if (jobs.empty())
	{
		return nullptr;
	}

	if (jobs.back()->CanRun())
	{
		auto ret = std::move(jobs.back());
		jobs.pop_back();
		return ret;
	}

	for (auto& job : jobs)
	{
		if (job->CanRun())
		{
			std::swap(job, jobs.back());
			auto ret = std::move(jobs.back());
			jobs.pop_back();
			return ret;
		}
	}

	return nullptr;
}

bool CAM::JobPool::Empty()
{
	std::shared_lock<std::shared_mutex> lock(sMutex);
	return jobs.empty();
}

bool CAM::JobPool::AnyRunnableJobs()
{
	std::shared_lock<std::shared_mutex> lock(sMutex);
	if (jobs.empty())
	{
		return false;
	}

	for (auto& job : jobs)
	{
		if(job->CanRun())
		{
			return true;
		}
	}

	return false;
}

