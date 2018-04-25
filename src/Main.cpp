/*
 * Copyright (C) 2018 Hal Gentz
 *
 * This file is part of CAM-RE.
 *
 * CAM-RE is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * Bash is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * CAM-RE. If not, see <http://www.gnu.org/licenses/>.
 */

#include "Main.hpp"

void CAM::Main::Start()
{
	auto myWorkerUni = std::make_unique<CAM::Jobs::Worker>(&wp, false);
	auto myWorker = myWorkerUni.get();
	wp.AddWorker(std::move(myWorkerUni));

	for (size_t i = 0; i < Config::ThreadCount - 1; ++i)
	{
		wp.AddWorker(std::make_unique<CAM::Jobs::Worker>(&wp, true));
	}

	wp.StartWorkers();

	/*
	 * [Init] -> [FrameStart] -> [Done]
	 *						 \-> [[M]DoneMain]
	 */

	using namespace std::placeholders;
	auto fsJob = wp.GetJob
	(
		std::bind(&Main::FrameStart, this, _1, _2, _3, _4),
		nullptr,
		2,
		false
	);

	auto iJob = wp.GetJob
	(
		std::bind(&Main::Init, this, _1, _2, _3, _4),
		nullptr,
		1,
		false
	);

	fsJob->DependsOn(iJob.get());
	if (!wp.SubmitJob(std::move(iJob))) { throw std::runtime_error("Could not submit job\n"); }

	auto dJob = wp.GetJob
	(
		std::bind(&Main::Done, this, _1, _2, _3, _4),
		nullptr,
		0,
		false
	);
	auto dmJob = wp.GetJob
	(
		std::bind(&Main::DoneMain, this, _1, _2, _3, _4),
		nullptr,
		0,
		true
	);

	dJob->DependsOn(fsJob.get());
	dmJob->DependsOn(fsJob.get());

	if (!wp.SubmitJob(std::move(fsJob))) { throw std::runtime_error("Could not submit job\n"); }
	if (!wp.SubmitJob(std::move(dJob))) { throw std::runtime_error("Could not submit job\n"); }
	if (!wp.SubmitJob(std::move(dmJob))) { throw std::runtime_error("Could not submit job\n"); }



	myWorker->WorkerRoutine();
}

void CAM::Main::Init
(
	void* /*userData*/,
	CAM::Jobs::WorkerPool* /*wp*/,
	size_t /*thread*/,
	CAM::Jobs::Job* thisJob
)
{
	renderer = std::make_unique<Renderer::Renderer>(&wp, thisJob);
}

void CAM::Main::FrameStart
(
	void* /*userData*/,
	CAM::Jobs::WorkerPool* /*wp*/,
	size_t /*thread*/,
	CAM::Jobs::Job* thisJob
)
{
	/* If renderer->ShouldContinue
	 * [renderer->DoFrame] -> [FrameStart] -> *
	 */

	if (!renderer->ShouldContinue())
	{
		return;
	}

	using namespace std::placeholders;
	auto fsJob = wp.GetJob
	(
		std::bind(&Main::FrameStart, this, _1, _2, _3, _4),
		nullptr,
		0,
		false
	);

	auto dfJob = wp.GetJob
	(
		std::bind(&Renderer::Renderer::DoFrame, renderer.get(), _1, _2, _3, _4),
		nullptr,
		1,
		false
	);

	fsJob->DependsOn(dfJob.get());
	if (!wp.SubmitJob(std::move(dfJob))) { throw std::runtime_error("Could not submit job\n"); }

	fsJob->SameThingsDependOnMeAs(thisJob);
	if (!wp.SubmitJob(std::move(fsJob))) { throw std::runtime_error("Could not submit job\n"); }
}

void CAM::Main::Done
(
	void* /*userData*/,
	CAM::Jobs::WorkerPool* /*wp*/,
	size_t thread,
	CAM::Jobs::Job* /*thisJob*/
)
{
	printf("Thread %zu says, \"thanks for playing.\"\n", thread);
}

void CAM::Main::DoneMain
(
	void* /*userData*/,
	CAM::Jobs::WorkerPool* /*wp*/,
	size_t thread,
	CAM::Jobs::Job* /*thisJob*/
)
{
	printf("Main thread %zu says, \"thanks for playing.\"\n", thread);
}

int main()
{
	CAM::Main m;
	m.Start();

	return 0;
}
