#ifndef CAM_JOBPOOL_HPP
#define CAM_JOBPOOL_HPP

#include <vector>
#include <mutex>
#include <shared_mutex>
#include <thread>
#include <cstdint>

namespace CAM
{
class Job;

class JobPool
{
	public:
	void SubmitJob(std::unique_ptr<Job> job);
	std::unique_ptr<Job> PullJob();

	bool Empty();
	bool AnyRunnableJobs();

	private:
	std::shared_mutex sMutex;
	std::vector<std::unique_ptr<Job>> jobs;
};
}

#endif
