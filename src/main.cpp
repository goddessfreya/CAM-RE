#include "WorkerPool.hpp"
#include "Job.hpp"

#include <chrono>
#include <cstdio>

#include <iostream>

const int threadCount = std::thread::hardware_concurrency();
const int jobTime = 200;
const int jobs = 1000;
const int jobSets = 1024;
const int minStepSize = 0;
const int maxStepSize = 50;

/*
 * Makes a lot of parellel jobs
 *
 * [0]
 * ...
 * [jobs - 1]
 *
 */
void parelel_jobs(void*, CAM::WorkerPool* wp, size_t)
{
	for (int i = 0; i < jobs; ++i)
	{
		wp->SubmitJob(
			wp->GetJob
			(
				[i] (void*, CAM::WorkerPool*, size_t)
				{
					volatile int z = 10 * jobTime + (i % threadCount * 2) * jobTime;
					volatile int s = 0;
					static CAM::ThreadSafeRandomNumberGenerator<int> ranGen;
					while (z > 4)
					{
						z -= ranGen(minStepSize, maxStepSize);
						++s;
					}
				},
				nullptr,
				0
			)
		);
	}
}

/*
 * Makes a lot of jobs which depend on the previous
 * [jobs - 1]->...->[0]
 */
void dep_chain_jobs(void*, CAM::WorkerPool* wp, size_t)
{
	std::unique_ptr<CAM::Job> prevJob = nullptr;
	for (int i = 0; i < jobs; ++i)
	{
		auto job = wp->GetJob
		(
			[i] (void*, CAM::WorkerPool*, size_t)
			{
				volatile int z = 10 * jobTime + (i % threadCount * 2) * jobTime;
				volatile int s = 0;
				static CAM::ThreadSafeRandomNumberGenerator<int> ranGen;
				while (z > 4)
				{
					z -= ranGen(minStepSize, maxStepSize);
					++s;
				}
			},
			nullptr,
			1
		);
		prevJob->DependsOn(job.get());


		wp->SubmitJob(
			std::move(prevJob)
		);

		prevJob = std::move(job);
	}
	wp->SubmitJob(
		std::move(prevJob)
	);
}

/*
 * Makes a lot of jobs which all depend on one job and have one job which depends on them
 *    />[   1]--v
 * [0]->[....]->[jobs + 1]
 *    \>[jobs]--^
 */
void shallow_dep_chain(void*, CAM::WorkerPool* wp, size_t)
{
	std::vector<std::unique_ptr<CAM::Job>> jobsToSubmit;
	jobsToSubmit.resize(jobs + 2);

	jobsToSubmit[0] = std::make_unique<CAM::Job>
	(
		[] (void*, CAM::WorkerPool*, size_t)
		{
			volatile int z = 10 * jobTime + (0 % threadCount * 2) * jobTime;
			volatile int s = 0;
			static CAM::ThreadSafeRandomNumberGenerator<int> ranGen;
			while (z > 4)
			{
				z -= ranGen(minStepSize, maxStepSize);
				++s;
			}
		},
		nullptr,
		jobs
	);

	jobsToSubmit[jobs + 1] = std::make_unique<CAM::Job>
	(
		[] (void*, CAM::WorkerPool*, size_t)
		{
			volatile int z = 10 * jobTime + ((jobs + 1) % threadCount * 2) * jobTime;
			int s = 0;
			static CAM::ThreadSafeRandomNumberGenerator<int> ranGen;
			while (z > 4)
			{
				z -= ranGen(minStepSize, maxStepSize);
				++s;
			}
		},
		nullptr,
		0
	);

	for (int i = 1; i < jobs + 1; ++i)
	{
		jobsToSubmit[i] = std::make_unique<CAM::Job>
		(
			[i] (void*, CAM::WorkerPool*, size_t)
			{
				volatile int z = 10 * jobTime + (i % threadCount * 2) * jobTime;
				int s = 0;
				static CAM::ThreadSafeRandomNumberGenerator<int> ranGen;
				while (z > 4)
				{
					z -= ranGen(minStepSize, maxStepSize);
					++s;
				}
			},
			nullptr,
			1
		);
		jobsToSubmit[i]->DependsOn(jobsToSubmit[0].get());
		jobsToSubmit[i]->DependsOnMe(jobsToSubmit[jobs + 1].get());
	}

	for (auto& job : jobsToSubmit)
	{
		wp->SubmitJob(std::move(job));
	}
}

int main()
{
	CAM::WorkerPool wp;
	for (int i = 0; i < threadCount - 1; ++i)
	{
		wp.AddWorker(std::make_unique<CAM::Worker>(&wp, true));
	}

	auto myWorkerUni = std::make_unique<CAM::Worker>(&wp, false);
	auto myWorker = myWorkerUni.get();
	wp.AddWorker(std::move(myWorkerUni));

	wp.StartWorkers();

	auto a = wp.GetJob(&dep_chain_jobs, nullptr, 0);
	printf("Each unique_ptr<Job> is: %zu and each Job is: %zu\n", sizeof(a), sizeof(*a));

	for (int i = 0; i < jobSets; ++i)
	{
		wp.SubmitJob(wp.GetJob(&dep_chain_jobs, nullptr, 0));
		wp.SubmitJob(wp.GetJob(&parelel_jobs, nullptr, 0));
		wp.SubmitJob(wp.GetJob(&shallow_dep_chain, nullptr, 0));
	}

	myWorker->WorkerRoutine();

	return 0;
}
