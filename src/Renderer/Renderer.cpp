#include "Renderer.hpp"

CAM::Renderer::Renderer::Renderer
(
	Jobs::WorkerPool* wp,
	Jobs::Job* thisJob
) : wp(wp)
{
	/*
	 * [[M]SDLWindow Lambda] -> *
	 */
	auto deps = thisJob->GetDepsOnMe();

	auto sdlJob = wp->GetJob
	(
		[this](void*, Jobs::WorkerPool* wp, size_t thread, Jobs::Job* thisJob)
		{
			assert(thread == 0);
			window = std::make_unique<SDLWindow>(wp, thisJob, this);
		},
		nullptr,
		deps.first.size(),
		true // main thread only
	);

	for (auto& deps : deps.first)
	{
		sdlJob->DependsOnMe(deps);
	}

	if (!wp->SubmitJob(std::move(sdlJob))) { throw std::runtime_error("Could not submit job\n"); }
}

void CAM::Renderer::Renderer::DoFrame
(
	void* /*userData*/,
	CAM::Jobs::WorkerPool* wp,
	size_t /*thread*/,
	CAM::Jobs::Job* thisJob
)
{
	/*
	 * [[M]window->HandleEvents] -> *
	 */
	auto deps = thisJob->GetDepsOnMe();

	using namespace std::placeholders;
	auto sdlJob = wp->GetJob
	(
		std::bind(&SDLWindow::HandleEvents, window.get(), _1, _2, _3, _4),
		nullptr,
		deps.first.size(),
		true // main thread only
	);

	for (auto& deps : deps.first)
	{
		sdlJob->DependsOnMe(deps);
	}

	if (!wp->SubmitJob(std::move(sdlJob))) { throw std::runtime_error("Could not submit job\n"); }
}
bool CAM::Renderer::Renderer::ShouldContinue() { return window->ShouldContinue(); }
