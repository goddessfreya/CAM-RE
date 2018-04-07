#ifndef CAM_JOB_HPP
#define CAM_JOB_HPP

#include <vector>
#include <atomic>
#include <cstdint>
#include <functional>

#include "JobPool.hpp"

namespace CAM
{
class JobPool;

class Job
{
	public:
	using JobFunc = std::function<void(void* userData, size_t thread)>;
	inline Job(JobFunc job, void* userData) : job(job), userData(userData) {}
	inline std::unique_ptr<Job> DoJob(size_t thread)
	{
		if (CanRun())
		{
			job(userData, thread);
			Job* toRun = nullptr;
			for (auto& dep : dependsOnMe)
			{
				--(dep->dependencesIncomplete);
				if (dep->dependencesIncomplete == 0 && dep->owner != nullptr)
				{
					if (toRun != nullptr)
					{
						dep->owner->MakeRunnable(dep);
					}
					else
					{
						toRun = dep;
					}
				}
			}

			if (toRun != nullptr)
			{
				return toRun->owner->PullDepJob(toRun);
			}

			return nullptr;
		}
		throw std::logic_error("We shouldn't do jobs we can't run.");
	}
	inline bool CanRun() { return dependencesIncomplete == 0; }
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
	JobPool* owner;
	JobFunc job;
	void* userData;
	std::atomic<int> dependencesIncomplete = 0;

	std::vector<Job*> dependsOnMe;
};
}

#endif
