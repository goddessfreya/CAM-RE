#ifndef CAM_JOB_HPP
#define CAM_JOB_HPP

#include <vector>
#include <atomic>
#include <cstdint>
#include <functional>

namespace CAM
{
class Job
{
	public:
	using JobFunc = std::function<void(void* userData)>;
	inline Job(JobFunc job, void* userData) : job(job), userData(userData) {}
	inline void DoJob()
	{
		if (CanRun())
		{
			job(userData);
			for (auto& dep : dependsOnMe)
			{
				--dep->dependencesIncomplete;
			}
		}
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

	private:
	JobFunc job;
	void* userData;
	std::atomic<int> dependencesIncomplete = 0;

	std::vector<Job*> dependsOnMe;
};
}

#endif
