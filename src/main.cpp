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

#include "Oren-lang/Lexer.hpp"
#include "Oren-lang/Parser.hpp"
#include "Oren-lang/IRBuilder.hpp"

#include <experimental/filesystem>

//const int threadCount = std::thread::hardware_concurrency() * 2 + 1;
const int threadCount = 1;

namespace OL
{
namespace fs = std::experimental::filesystem;
class Main
{
	public:
	Main() : lexer(&parser), parser(&lexer, &irBuilder), irBuilder(&parser) {}
	void Start();
	static void Done
	(
		void* userData,
		CAM::WorkerPool* wp,
		size_t thread,
		CAM::Job* thisJob
	);

	private:
	Lexer lexer;
	Parser parser;
	IRBuilder irBuilder;
};
}

void OL::Main::Start()
{
	CAM::WorkerPool wp;
	irBuilder.BuildFile("ttttttext!!!!!!!!!!!!!!!", nullptr, &wp, 20, nullptr);

	for (int i = 0; i < threadCount - 1; ++i)
	{
		wp.AddWorker(std::make_unique<CAM::Worker>(&wp, true));
	}

	auto myWorkerUni = std::make_unique<CAM::Worker>(&wp, false);
	auto myWorker = myWorkerUni.get();
	wp.AddWorker(std::move(myWorkerUni));

	wp.StartWorkers();

	for (auto& path : OL::fs::directory_iterator("./osources"))
	{
		if (OL::fs::is_regular_file(path))
		{
			lexer.SubmitFile(path.path().string());
		}
	}

	using namespace std::placeholders;
	auto lJob = wp.GetJob
	(
		std::bind(&Lexer::Start, &lexer, _1, _2, _3, _4),
		nullptr,
		1
	);

	auto dJob = wp.GetJob(&Main::Done, this, 0);
	dJob->DependsOn(lJob.get());
	wp.SubmitJob(std::move(lJob));

	wp.SubmitJob(std::move(dJob));

	myWorker->WorkerRoutine();
}

void OL::Main::Done
(
	void* /*userData*/,
	CAM::WorkerPool* /*wp*/,
	size_t thread,
	CAM::Job* /*thisJob*/
)
{
	printf("%zu: Main done.\n", thread);
}

int main()
{
	OL::Main m;
	m.Start();

	return 0;
}
