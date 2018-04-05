#ifndef CAM_JOB_HPP
#define CAM_JOB_HPP

#include <vector>
#include <cstdint>
#include <functional>

namespace CAM
{
class Job
{
	public:
	using JobFunc = std::function<void(void* userData)>;
	inline Job(JobFunc job, void* userData) : job(job), userData(userData) {}
	inline void DoJob() { job(userData); }

	private:
	JobFunc job;
	void* userData;
};
}

#endif
