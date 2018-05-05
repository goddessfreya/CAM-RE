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

#include "JobPool.hpp"
#include "Job.hpp"
#include "WorkerPool.hpp"
#include "../Utils/Assert.hpp"

CAM::Jobs::JobPool::JobPool(WorkerPool* wp) : wp(wp)
{}

void CAM::Jobs::JobPool::SubmitJob(std::unique_ptr<Job> job)
{
	if (job->CanRun())
	{
		std::unique_lock<std::shared_mutex> lock(jobsMutex);

		if (!jobs.empty())
		{
			wp->WakeUpThreads(1);
		}

		jobs.push_back(std::move(job));
		jobs.back()->SetOwner(this);
	}
	else
	{
		std::unique_lock<std::shared_mutex> lock(jobsWithUnmetDepsMutex);
		jobsWithUnmetDeps.push_back(std::move(job));
		jobsWithUnmetDeps.back()->SetOwner(this);
	}
}

std::unique_ptr<CAM::Jobs::Job> CAM::Jobs::JobPool::PullJob()
{
	std::unique_lock<std::shared_mutex> lock(jobsMutex);
	if (jobs.empty())
	{
		return nullptr;
	}

	jobs.back()->SetOwner(nullptr);
	auto ret = std::move(jobs.back());
	jobs.pop_back();
	return ret;
}

bool CAM::Jobs::JobPool::Empty() const
{
	std::shared_lock<std::shared_mutex> lock1(jobsMutex, std::defer_lock);
	std::shared_lock<std::shared_mutex> lock2(jobsWithUnmetDepsMutex, std::defer_lock);
	std::lock(lock1, lock2);

	return jobs.empty() && jobsWithUnmetDeps.empty();
}

bool CAM::Jobs::JobPool::NoRunnableJobs() const
{
	std::shared_lock<std::shared_mutex> lock(jobsMutex);
	return jobs.empty();
}

size_t CAM::Jobs::JobPool::RunnableJobsLeft() const
{
	std::shared_lock<std::shared_mutex> lock(jobsMutex);
	return jobs.size();
}

void CAM::Jobs::JobPool::MakeRunnable(Job* job)
{
	ASSERT(job->CanRun(), "We should only be making runnable jobs runnable.");
	SubmitJob(PullDepJob(job));
}

std::unique_ptr<CAM::Jobs::Job> CAM::Jobs::JobPool::PullDepJob(Job* job)
{
	ASSERT(job->CanRun(), "We should only be pulling runnable jobs out of the jobsWithUnmetDeps vector.");
	std::shared_lock<std::shared_mutex> lock(jobsWithUnmetDepsMutex);
	job->SetOwner(nullptr);

	auto e = std::find_if
	(
		std::begin(jobsWithUnmetDeps),
		std::end(jobsWithUnmetDeps),
		[&job] (const std::unique_ptr<Job>& a) -> bool
		{
			return a.get() == job;
		}
	);

	ASSERT(e != std::end(jobsWithUnmetDeps), "We should only be pulling jobs out of the jobsWithUnmetDeps vector if they are in it. Likely a sync error with the owner variable.");

	std::unique_ptr<Jobs::Job> ret = nullptr;
	std::swap(*e, ret);

	ASSERT(ret != nullptr, "The job we pulled shouldn't be a nullptr");

	while (!jobsWithUnmetDeps.empty() && jobsWithUnmetDeps.back() == nullptr)
	{
		lock.unlock();
		{
			std::unique_lock<std::shared_mutex> lock2(jobsWithUnmetDepsMutex);
			jobsWithUnmetDeps.pop_back();
		}
		lock.lock();
	}

	return ret;
}
