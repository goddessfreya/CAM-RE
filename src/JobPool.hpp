#ifndef CAM_JOBPOOL_HPP
#define CAM_JOBPOOL_HPP

#include <vector>
#include <mutex>
#include <shared_mutex>
#include <thread>
#include <algorithm>
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
	bool NoRunnableJobs();

	void MakeRunnable(Job* job);

	std::unique_ptr<Job> PullDepJob(Job* job);

	private:
	std::shared_mutex jobsMutex;
	std::shared_mutex jobsWithUnmetDepsMutex;
	std::vector<std::unique_ptr<Job>> jobs;
	std::vector<std::unique_ptr<Job>> jobsWithUnmetDeps;
};
}

#endif
