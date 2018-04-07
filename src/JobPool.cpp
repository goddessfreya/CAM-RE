#include "JobPool.hpp"
#include "Job.hpp"
#include <cassert>

void CAM::JobPool::SubmitJob(std::unique_ptr<Job> job)
{
	job->SetOwner(this);
	if (job->CanRun())
	{
		std::unique_lock<std::shared_mutex> lock(jobsMutex);
		jobs.push_back(std::move(job));
	}
	else
	{
		std::unique_lock<std::shared_mutex> lock(jobsWithUnmetDepsMutex);
		jobsWithUnmetDeps.push_back(std::move(job));
	}
}

std::unique_ptr<CAM::Job> CAM::JobPool::PullJob()
{
	std::unique_lock<std::shared_mutex> lock(jobsMutex);
	if (jobs.empty())
	{
		return nullptr;
	}

	auto ret = std::move(jobs.back());
	ret->SetOwner(nullptr);
	jobs.pop_back();
	return ret;
}

bool CAM::JobPool::Empty()
{
	std::shared_lock<std::shared_mutex> lock1(jobsMutex, std::defer_lock);
	std::shared_lock<std::shared_mutex> lock2(jobsWithUnmetDepsMutex, std::defer_lock);
	std::lock(lock1, lock2);

	return jobs.empty() == 0 && jobsWithUnmetDeps.empty() == 0;
}

bool CAM::JobPool::AnyRunnableJobs()
{
	std::shared_lock<std::shared_mutex> lock(jobsMutex);
	return jobs.empty() == 0;
}

void CAM::JobPool::MakeRunnable(Job* job)
{
	std::unique_lock<std::shared_mutex> lock1(jobsMutex, std::defer_lock);
	std::unique_lock<std::shared_mutex> lock2(jobsWithUnmetDepsMutex, std::defer_lock);
	std::lock(lock1, lock2);

	auto e = std::find_if
	(
		std::begin(jobsWithUnmetDeps),
		std::end(jobsWithUnmetDeps),
		[&job] (const std::unique_ptr<Job>& a) -> bool
		{
			return a.get() == job;
		}
	);

	assert(e != std::end(jobsWithUnmetDeps));
	std::swap(*e, jobsWithUnmetDeps.back());
	jobs.push_back(std::move(jobsWithUnmetDeps.back()));
	jobsWithUnmetDeps.pop_back();
}

std::unique_ptr<CAM::Job> CAM::JobPool::PullDepJob(Job* job)
{
	std::unique_lock<std::shared_mutex> lock(jobsWithUnmetDepsMutex);

	auto e = std::find_if
	(
		std::begin(jobsWithUnmetDeps),
		std::end(jobsWithUnmetDeps),
		[&job] (const std::unique_ptr<Job>& a) -> bool
		{
			return a.get() == job;
		}
	);

	assert(e != std::end(jobsWithUnmetDeps));
	std::swap(*e, jobsWithUnmetDeps.back());
	auto ret = std::move(jobsWithUnmetDeps.back());
	jobsWithUnmetDeps.pop_back();
	return ret;
}
