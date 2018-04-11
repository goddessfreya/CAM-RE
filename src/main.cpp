#include "Oren-lang/Lexer.hpp"

#include <experimental/filesystem>

const int threadCount = std::thread::hardware_concurrency() * 2 + 1;

namespace OL
{
namespace fs = std::experimental::filesystem;
class Main
{
	public:
	void Start();
	static void Done(void* userData, CAM::WorkerPool* wp, size_t thread, CAM::Job* thisJob);

	private:
	Lexer lexer;
};
}

void OL::Main::Start()
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

	for (auto& path : OL::fs::directory_iterator("./osources"))
	{
		if (OL::fs::is_regular_file(path))
		{
			lexer.SubmitFile(path.path().string());
		}
	}

	auto lJob = wp.GetJob(&Lexer::Start, &lexer, 1);

	auto dJob = wp.GetJob(&Main::Done, this, 0);
	dJob->DependsOn(lJob.get());
	wp.SubmitJob(std::move(lJob));

	wp.SubmitJob(std::move(dJob));

	myWorker->WorkerRoutine();
}

void OL::Main::Done(void* /*userData*/, CAM::WorkerPool* /*wp*/, size_t thread, CAM::Job* /*thisJob*/)
{
	printf("%zu: Main done.\n", thread);
}

int main()
{
	OL::Main m;
	m.Start();

	return 0;
}
