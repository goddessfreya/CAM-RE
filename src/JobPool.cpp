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

	auto ret = std::move(jobs.back());
	jobs.pop_back();
	return ret;
}

bool CAM::JobPool::empty()
{
	std::shared_lock<std::shared_mutex> lock(sMutex);
	return jobs.empty();
}

