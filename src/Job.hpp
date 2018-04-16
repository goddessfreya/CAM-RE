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

/*
 * This is a simple job class.
 *
 * Please do not allocate it directly. Instead, request one from your
 * WorkerPool.
 */

#ifndef CAM_JOB_HPP
#define CAM_JOB_HPP

#include <vector>
#include <atomic>
#include <cstdint>
#include <functional>
#include <cassert>
#include <new>

#include "Utils/CountedMutex.hpp"
#include "JobPool.hpp"
#include "Utils/Aligner.tpp"

namespace CAM
{
class WorkerPool;
class Job;

struct JobD
{
	using JobFunc = std::function<void(void* userData, WorkerPool* wp, size_t thread, Job* thisJob)>;
	std::atomic<JobPool*> owner = nullptr;
	JobFunc job;
	void* userData;
	int dependencesIncomplete = 0;
	std::vector<Job*> dependsOnMe;
	CountedMutex dependencesIncompleteMutex;
	bool mainThreadOnly;
};

class Job : private Aligner<JobD>
{
	public:
	inline Job(JobFunc job, void* userData, size_t depsOnMe, bool mainThreadOnly) { Reset(job, userData, depsOnMe, mainThreadOnly); }
	inline Job() { }
	inline void Reset(JobFunc job, void* userData, size_t depsOnMe, bool mainThreadOnly)
	{
		this->job = job;
		this->userData = userData;
		owner = nullptr;
		dependencesIncomplete = 0;
		dependsOnMe.clear();
		dependsOnMe.reserve(depsOnMe);
		this->mainThreadOnly = mainThreadOnly;

		assert(dependencesIncompleteMutex.LockersLeft() == 0);
		assert(dependencesIncompleteMutex.UniqueLocked() == false);
	}

	inline std::unique_ptr<Job> DoJob(WorkerPool* wp, size_t thread)
	{
		if (CanRun())
		{
			job(userData, wp, thread, this);
			Job* toRun = nullptr;
			for (auto& dep : dependsOnMe)
			{
				// TODO: Make it so we go do other jobs then come back.
				while (dep->owner == nullptr)
				{
					std::this_thread::yield();
				}

				std::unique_lock<std::mutex> lock (dep->dependencesIncompleteMutex);
				--(dep->dependencesIncomplete);
				if (dep->dependencesIncomplete == 0 && dep->owner != nullptr)
				{
					if (toRun != nullptr)
					{
						static_cast<JobPool*>(dep->owner)->MakeRunnable(dep);
					}
					else
					{
						toRun = dep;
					}
				}
			}

			if (toRun != nullptr)
			{
				return static_cast<JobPool*>(toRun->owner)->PullDepJob(toRun);
			}

			return nullptr;
		}
		throw std::logic_error("We shouldn't do jobs we can't run.");
	}
	inline bool CanRun() const { return dependencesIncomplete == 0; }

	// TODO: Make events a type of dependency
	inline void DependsOn(Job* other)
	{
		other->DependsOnMe(this);
	}
	inline void DependsOnMe(Job* other)
	{
		if (other == nullptr)
		{
			return;
		}
		++other->dependencesIncomplete;
		dependsOnMe.push_back(other);
	}

	inline void SetOwner(JobPool* owner) { this->owner = owner; }

	inline size_t NumberOfDepsOnMe() const
	{
		return dependsOnMe.size();
	}

	inline const std::vector<Job*>& GetDepsOnMe() const { return dependsOnMe; }
	inline bool MainThreadOnly() const { return mainThreadOnly; }
};
}

#endif
