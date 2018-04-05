#include "ThreadPool.hpp"
#include "Job.hpp"
#include <cstdio>
#include <cstdlib> // TODO: REMOVE, FOR DEBUG ONLY

const uint32_t threadCount = 5;

void sub_jobs(void* tpPtr)
{
	auto tp = static_cast<CAM::ThreadPool*>(tpPtr);
	uint32_t pool = 0;
	for (uint32_t i = 0; i < 1000; ++i)
	{
		tp->SubmitJob(
			std::make_unique<CAM::Job>
			(
				[i] (void*)
				{
					volatile uint32_t z = 1000000 + (i % threadCount) * 100000;
					z /= 100;
					while (z > 4)
					{
						z -= rand() % 3;
					}
					printf("Job %i is done\n", i);
				},
				nullptr
			),
			pool
		);
	}
}

int main()
{
	CAM::ThreadPool tp(threadCount);

	uint32_t pool = 0;
	for (uint32_t i = 0; i < 10; ++i)
	{
		tp.SubmitJob(std::make_unique<CAM::Job>(&sub_jobs, &tp), pool);
	}

	tp.WaitTillNoJobs();
	tp.WaitTillIdle();

	return 0;
}
