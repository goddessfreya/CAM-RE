#include "ThreadPool.hpp"
#include "Job.hpp"

CAM::ThreadPool::ThreadPool(uint32_t numberOfThreads)
{
	std::unique_lock<std::shared_mutex> lock(idleMutex);
	threads.reserve(numberOfThreads);
	runningThreads = numberOfThreads;
	for (uint32_t i = 0; i < numberOfThreads; ++i)
	{
		threads.push_back(std::make_unique<Thread>(this));
	}
}

CAM::ThreadPool::~ThreadPool()
{
	// We cannot kill any Thread before any other Thread's thread dies else
	// shenanigans happen. So we wait for all of them to die first.
	for (auto& thread : threads)
	{
		thread->RequestInactivity();
	}

	while (runningThreads != 0)
	{
		std::this_thread::yield();
	}
}

void CAM::ThreadPool::RegisterInactvitiy()
{
	--runningThreads;
}

bool CAM::ThreadPool::SubmitJob(std::unique_ptr<Job> job, uint32_t& submitPool)
{
	if (submitPool >= threads.size())
	{
		submitPool = 0;
	}

	bool first = true;
	do
	{
		if (submitPool >= threads.size())
		{
			if (!first)
			{
				return false;
			}
			submitPool = 0;
			first = false;
		}

		if (threads[submitPool] != nullptr)
		{
			threads[submitPool]->SubmitJob(std::move(job));
			++submitPool;
			return true;
		}
		++submitPool;
	} while (true);
}

std::unique_ptr<CAM::Job> CAM::ThreadPool::PullJob(uint32_t& pullPool)
{
	if (pullPool >= threads.size())
	{
		pullPool = 0;
	}

	bool first = false;
	do
	{
		if (pullPool >= threads.size())
		{
			if (first)
			{
				return nullptr;
			}
			pullPool = 0;
			first = true;
		}
		if (threads[pullPool] != nullptr && !threads[pullPool]->JobPoolEmpty())
		{
			auto ret = threads[pullPool]->PullJob();

			++pullPool;
			return ret;
		}
		++pullPool;
	} while (true);
}
