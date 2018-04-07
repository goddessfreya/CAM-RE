#include "WorkerPool.hpp"
#include "Job.hpp"

#include <chrono>
#include <cstdio>

#include <iostream>

const int threadCount = 4;
const int jobTime = 1000;
const int jobs = 300;
const int jobSets = 100;
const int minStepSize = 0;
const int maxStepSize = 5;

/*
 * Makes a lot of parellel jobs
 *
 * [0]
 * ...
 * [jobs - 1]
 *
 */
void parelel_jobs(void* tpPtr, size_t thread)
{
	static std::atomic<int> c = 0;
	auto tc = c++;
	auto tp = static_cast<CAM::WorkerPool*>(tpPtr);
	printf("T %zu: Par %i: Populating\n", thread, tc);
	for (int i = 0; i < jobs; ++i)
	{
		tp->SubmitJob(
			std::make_unique<CAM::Job>
			(
				[i, tc] (void*, size_t thread)
				{
					volatile int z = 10 * jobTime + (i % threadCount * 2) * jobTime;
					int s = 0;
					static CAM::ThreadSafeRandomNumberGenerator<int> ranGen;
					while (z > 4)
					{
						z -= ranGen(minStepSize, maxStepSize);
						++s;
					}
					printf("T %zu: Par %i: Job %i done in %i steps\n", thread, tc, i, s);
				},
				nullptr
			)
		);
	}
	printf("T %zu: Par %i: Done\n", thread, tc);
}

/*
 * Makes a lot of jobs which depend on the previous
 * [jobs - 1]->...->[0]
 */
void dep_chain_jobs(void* tpPtr, size_t thread)
{
	static std::atomic<int> c = 0;
	auto tc = c++;
	auto tp = static_cast<CAM::WorkerPool*>(tpPtr);
	std::unique_ptr<CAM::Job> prevJob = nullptr;
	printf("T %zu: Dep %i: Populating\n", thread, tc);
	for (int i = 0; i < jobs; ++i)
	{
		auto job = std::make_unique<CAM::Job>
		(
			[i, tc] (void*, size_t thread)
			{
				volatile int z = 10 * jobTime + (i % threadCount * 2) * jobTime;
				int s = 0;
				static CAM::ThreadSafeRandomNumberGenerator<int> ranGen;
				while (z > 4)
				{
					z -= ranGen(minStepSize, maxStepSize);
					++s;
				}
				printf("T %zu: Dep %i: Job %i done in %i steps\n", thread, tc, i, s);
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
	printf("T %zu: Dep %i: Done\n", thread, tc);
}

/*
 * Makes a lot of jobs which all depend on one job and have one job which depends on them
 *    />[   1]--v
 * [0]->[....]->[jobs + 1]
 *    \>[jobs]--^
 */
void shallow_dep_chain(void* tpPtr, size_t thread)
{
	static std::atomic<int> c = 0;
	auto tc = c++;
	auto tp = static_cast<CAM::WorkerPool*>(tpPtr);
	std::vector<std::unique_ptr<CAM::Job>> jobsToSubmit;
	jobsToSubmit.resize(jobs + 2);

	jobsToSubmit[0] = std::make_unique<CAM::Job>
	(
		[tc] (void*, size_t thread)
		{
			volatile int z = 10 * jobTime + (0 % threadCount * 2) * jobTime;
			int s = 0;
			static CAM::ThreadSafeRandomNumberGenerator<int> ranGen;
			while (z > 4)
			{
				z -= ranGen(minStepSize, maxStepSize);
				++s;
			}
			printf("T %zu: SDep %i: Job %i done in %i steps\n", thread, tc, 0, s);
		},
		nullptr
	);

	jobsToSubmit[jobs + 1] = std::make_unique<CAM::Job>
	(
		[tc] (void*, size_t thread)
		{
			volatile int z = 10 * jobTime + ((jobs + 1) % threadCount * 2) * jobTime;
			int s = 0;
			static CAM::ThreadSafeRandomNumberGenerator<int> ranGen;
			while (z > 4)
			{
				z -= ranGen(minStepSize, maxStepSize);
				++s;
			}
			printf("T %zu: SDep %i: Job %i done in %i steps\n", thread, tc, jobs + 1, s);
		},
		nullptr
	);

	printf("T %zu: SDep %i: Populating\n", thread, tc);
	for (int i = 1; i < jobs + 1; ++i)
	{
		jobsToSubmit[i] = std::make_unique<CAM::Job>
		(
			[i, tc] (void*, size_t thread)
			{
				volatile int z = 10 * jobTime + (i % threadCount * 2) * jobTime;
				int s = 0;
				static CAM::ThreadSafeRandomNumberGenerator<int> ranGen;
				while (z > 4)
				{
					z -= ranGen(minStepSize, maxStepSize);
					++s;
				}
				printf("T %zu: SDep %i: Job %i done in %i steps\n", thread, tc, i, s);
			},
			nullptr
		);
		jobsToSubmit[i]->DependsOn(jobsToSubmit[0].get());
		jobsToSubmit[i]->DependsOnMe(jobsToSubmit[jobs + 1].get());
	}

	for (auto& job : jobsToSubmit)
	{
		tp->SubmitJob(std::move(job));
	}

	printf("T %zu: SDep %i: Done\n", thread, tc);
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
		using namespace std::placeholders;
		tp.SubmitJob(std::make_unique<CAM::Job>(&dep_chain_jobs, &tp));
		tp.SubmitJob(std::make_unique<CAM::Job>(&parelel_jobs, &tp));
		tp.SubmitJob(std::make_unique<CAM::Job>(&shallow_dep_chain, &tp));
	}

	myWorker->WorkerRoutine();

	return 0;
}
