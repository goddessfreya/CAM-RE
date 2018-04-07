#include "WorkerPool.hpp"
#include "Job.hpp"

#include <chrono>
#include <cstdio>

const int threadCount = 4;
const int jobTime = 1000;
const int jobs = 300;
const int jobSets = 10;
const int minStepSize = 0;
const int maxStepSize = 5;

void parelel_jobs(void* tpPtr)
{
	static std::atomic<int> c = 0;
	auto tc = c++;
	auto tp = static_cast<CAM::WorkerPool*>(tpPtr);
	printf("Par %i: Populating\n", tc);
	for (int i = 0; i < jobs; ++i)
	{
		tp->SubmitJob(
			std::make_unique<CAM::Job>
			(
				[i, tc] (void*)
				{
					volatile int z = 10 * jobTime + (i % threadCount * 2) * jobTime;
					int s = 0;
					static CAM::ThreadSafeRandomNumberGenerator<int> ranGen;
					while (z > 4)
					{
						z -= ranGen(minStepSize, maxStepSize);
						++s;
					}
					printf("Par %i: Job %i done in %i steps\n", tc, i, s);
				},
				nullptr
			)
		);
	}
	printf("Par %i: Done\n", tc);
}

void dep_chain_jobs(void* tpPtr)
{
	static std::atomic<int> c = 0;
	auto tc = c++;
	auto tp = static_cast<CAM::WorkerPool*>(tpPtr);
	std::unique_ptr<CAM::Job> prevJob = nullptr;
	printf("Dep %i: Populating\n", tc);
	for (int i = 0; i < jobs; ++i)
	{
		auto job = std::make_unique<CAM::Job>
		(
			[i, tc] (void*)
			{
				volatile int z = 10 * jobTime + (i % threadCount * 2) * jobTime;
				int s = 0;
				static CAM::ThreadSafeRandomNumberGenerator<int> ranGen;
				while (z > 4)
				{
					z -= ranGen(minStepSize, maxStepSize);
					++s;
				}
				printf("Dep %i: Job %i done in %i steps\n", tc, i, s);
			},
			nullptr
		);
		prevJob->DependsOn(job.get());


		tp->SubmitJob(
			std::move(prevJob)
		);

		prevJob = std::move(job);
	}
	tp->SubmitJob(
		std::move(prevJob)
	);
	printf("Dep %i: Done\n", tc);
}

int main()
{
	CAM::WorkerPool tp;
	for (int i = 0; i < threadCount - 1; ++i)
	{
		tp.AddWorker(std::make_unique<CAM::Worker>(&tp, true));
	}

	auto myWorkerUni = std::make_unique<CAM::Worker>(&tp, false);
	auto myWorker = myWorkerUni.get();
	tp.AddWorker(std::move(myWorkerUni));

	tp.StartWorkers();

	for (int i = 0; i < jobSets; ++i)
	{
		tp.SubmitJob(std::make_unique<CAM::Job>(&dep_chain_jobs, &tp));
		tp.SubmitJob(std::make_unique<CAM::Job>(&parelel_jobs, &tp));
	}

	myWorker->WorkerRoutine();

	return 0;
}
