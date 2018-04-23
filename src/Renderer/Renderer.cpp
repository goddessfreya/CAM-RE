#include "Renderer.hpp"

CAM::Renderer::Renderer::Renderer
(
	Jobs::WorkerPool* wp,
	Jobs::Job* thisJob
) : wp(wp)
{
	/*
	 * [[M]SDLWindow Lambda] ----------------------------=> *
	 * [InitGlobalFuncs Lambda] -> [VKInstance Lambda] -/
	 */
	auto deps = thisJob->GetDepsOnMe();

	auto sdlJob = wp->GetJob
	(
		[this](void*, Jobs::WorkerPool* wp, size_t, Jobs::Job* thisJob)
		{
			window = std::make_unique<SDLWindow>(wp, thisJob, this);
		},
		nullptr,
		deps.first.size(),
		true // main thread only
	);

	for (auto& dep : deps.first)
	{
		sdlJob->DependsOnMe(dep);
	}
	if (!wp->SubmitJob(std::move(sdlJob))) { throw std::runtime_error("Could not submit job\n"); }

	auto igFNJob = wp->GetJob
	(
		[](void*, Jobs::WorkerPool*, size_t, Jobs::Job*)
		{
			if (!CAM::VKFN::InitGlobalFuncs()) { throw std::runtime_error("Could not init global funcs\n"); }
		},
		nullptr,
		1,
		false
	);

	auto vkInJob = wp->GetJob
	(
		[this](void*, Jobs::WorkerPool* wp, size_t, Jobs::Job* thisJob)
		{
			vkInstance = std::make_unique<VKInstance>(wp, thisJob, this);
		},
		nullptr,
		1,
		false
	);

	vkInJob->DependsOn(igFNJob.get());
	if (!wp->SubmitJob(std::move(igFNJob))) { throw std::runtime_error("Could not submit job\n"); }

	for (auto& dep : deps.first)
	{
		vkInJob->DependsOnMe(dep);
	}
	if (!wp->SubmitJob(std::move(vkInJob))) { throw std::runtime_error("Could not submit job\n"); }
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
