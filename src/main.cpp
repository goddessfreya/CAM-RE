#include "WorkerPool.hpp"
#include "Job.hpp"

#include <chrono>
#include <cstdio>

const int threadCount = 4;

void sub_jobs(void* tpPtr)
{
	auto tp = static_cast<CAM::WorkerPool*>(tpPtr);
	std::unique_ptr<CAM::Job> prevJob = nullptr;
	printf("Populating\n");
	for (int i = 0; i < 3; ++i)
	{
		auto job = std::make_unique<CAM::Job>
		(
			[i] (void*)
			{
				volatile int z = 1000000 + (i % threadCount) * 100000;
				z /= 100;
				static CAM::ThreadSafeRandomNumberGenerator<int> ranGen;
				while (z > 4)
				{
					z -= ranGen(0, 5);
				}
				fprintf(stderr, "Job %i is done in 0.5 sec\n", i);
				std::this_thread::sleep_for(std::chrono::milliseconds(500));
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
	for (int i = 0; i < threadCount - 1; ++i)
	{
		tp.AddWorker(std::make_unique<CAM::Worker>(&tp, true));
	}

	auto myWorkerUni = std::make_unique<CAM::Worker>(&tp, false);
	auto myWorker = myWorkerUni.get();
	tp.AddWorker(std::move(myWorkerUni));

	tp.StartWorkers();

	for (int i = 0; i < 1; ++i)
	{
		tp.SubmitJob(std::make_unique<CAM::Job>(&sub_jobs, &tp));
	}

	myWorker->WorkerRoutine();

	return 0;
}
