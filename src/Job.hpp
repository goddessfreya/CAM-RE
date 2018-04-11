#ifndef CAM_JOB_HPP
#define CAM_JOB_HPP

/*
 * This file is part of CAM-RE and is distributed under the GPLv3 License.
 * See LICENSE for more details.
 *
 * (C) 2018 Hal Gentz
 */

/*
 * This is a simple job class.
 *
 * Please do not allocate it directly. Instead, request one from your
 * WorkerPool.
 */

#include <vector>
#include <atomic>
#include <cstdint>
#include <functional>
#include <cassert>
#include <new>

#include "CountedMutex.hpp"
#include "JobPool.hpp"
#include "Aligner.tpp"

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
};

class Job : private Aligner<JobD>
{
	public:
	inline Job(JobFunc job, void* userData, size_t depsOnMe) { Reset(job, userData, depsOnMe); }
	inline Job() { }
	inline void Reset(JobFunc job, void* userData, size_t depsOnMe)
	{
		this->job = job;
		this->userData = userData;
		owner = nullptr;
		dependencesIncomplete = 0;
		dependsOnMe.clear();
		dependsOnMe.reserve(depsOnMe);

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
};
}

#endif
