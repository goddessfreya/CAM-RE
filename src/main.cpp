#include "WorkerPool.hpp"
#include "Job.hpp"
#include <cstdio>
#include <cstdlib> // TODO: REMOVE, FOR DEBUG ONLY

const int threadCount = 4;

void sub_jobs(void* tpPtr)
{
	auto tp = static_cast<CAM::WorkerPool*>(tpPtr);
	std::unique_ptr<CAM::Job> prevJob = nullptr;
	printf("Populating\n");
	for (int i = 0; i < 1000; ++i)
	{
		auto job = std::make_unique<CAM::Job>
		(
			[i] (void*)
			{
				volatile int z = 1000000 + (i % threadCount) * 100000;
				z /= 100;
				while (z > 4)
				{
					z -= rand() % 3;
				}
				printf("Job %i is done\n", i);
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
	printf("Done\n");
}

int main()
{
	CAM::WorkerPool tp;
	tp.RegisterInFlightOperation();
	for (int i = 0; i < threadCount - 1; ++i)
	{
		tp.AddWorker(std::make_unique<CAM::Worker>(&tp, true));
	}

	auto myWorkerUni = std::make_unique<CAM::Worker>(&tp, false);
	auto myWorker = myWorkerUni.get();
	tp.AddWorker(std::move(myWorkerUni));

	for (int i = 0; i < 1; i++)
	{
		tp.SubmitJob(std::make_unique<CAM::Job>(&sub_jobs, &tp));
	}
	tp.StartWorkers();
	tp.UnregisterInFlightOperation();
	myWorker->WorkerRoutine();
	tp.WaitTillNoJobs();

	return 0;
}
