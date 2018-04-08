#ifndef CAM_JOB_HPP
#define CAM_JOB_HPP

#include <vector>
#include <atomic>
#include <cstdint>
#include <functional>
#include <cassert>

#include "CountedMutex.hpp"
#include "JobPool.hpp"

namespace CAM
{
class JobPool;
class WorkerPool;

class Job
{
	public:
	using JobFunc = std::function<void(void* userData, WorkerPool* wp, size_t thread)>;
	inline Job(JobFunc job, void* userData) { Reset(job, userData); }
	inline Job() { }
	inline void Reset(JobFunc job, void* userData)
	{
		this->job = job;
		this->userData = userData;
		owner = nullptr;
		dependencesIncomplete = 0;
		dependsOnMe.clear();

		assert(dependencesIncompleteMutex.LockersLeft() == 0);
		assert(dependencesIncompleteMutex.UniqueLocked() == false);
	}

	inline std::unique_ptr<Job> DoJob(WorkerPool* wp, size_t thread)
	{
		if (CanRun())
		{
			job(userData, wp, thread);
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
	inline bool CanRun() { return dependencesIncomplete == 0; }

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

	private:
	std::atomic<JobPool*> owner = nullptr;
	JobFunc job;
	void* userData;
	int dependencesIncomplete = 0;
	CountedMutex dependencesIncompleteMutex;

	std::vector<Job*> dependsOnMe;
};
}

#endif
