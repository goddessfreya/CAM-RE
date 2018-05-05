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

#include "Renderer.hpp"

CAM::Renderer::Renderer::Renderer
(
	Jobs::WorkerPool* wp,
	Jobs::Job* thisJob
) : wp(wp)
{
	/*
	 * [[M]SDLWindow Lambda] --\     [[M]VKSurface Lambda] -V
	 * [InitGlobalFuncs Lambda] => [VKInstance Lambda] -^ [VKDevice Lambda] -\
	 * /---------------------------------------------------------------------/
	 * \-> [VKSurface::UpdateCaps] -> [VKSwapchain Lambda] -> *
	 */

	auto igFNJob = wp->GetJob
	(
		[](Jobs::WorkerPool*, size_t, Jobs::Job*)
		{
			if (!CAM::VKFN::InitGlobalFuncs()) { throw std::runtime_error("Could not init global funcs\n"); }
		},
		1,
		false
	);

	auto vkInJob = wp->GetJob
	(
		[this](Jobs::WorkerPool* wp, size_t, Jobs::Job* thisJob)
		{
			vkInstance = std::make_unique<VKInstance>(wp, thisJob, this);
		},
		1,
		false
	);

	vkInJob->DependsOn(igFNJob.get());
	if (!wp->SubmitJob(std::move(igFNJob))) { throw std::runtime_error("Could not submit job\n"); }

	auto sdlJob = wp->GetJob
	(
		[this](Jobs::WorkerPool* wp, size_t, Jobs::Job* thisJob)
		{
			window = std::make_unique<SDLWindow>(wp, thisJob, this);
		},
		1,
		true // main thread only
	);

	vkInJob->DependsOn(sdlJob.get());
	if (!wp->SubmitJob(std::move(sdlJob))) { throw std::runtime_error("Could not submit job\n"); }

	auto vkSJob = wp->GetJob
	(
		[this](Jobs::WorkerPool* wp, size_t, Jobs::Job* thisJob)
		{
			vkSurface = std::make_unique<VKSurface>(wp, thisJob, this);
		},
		1,
		true // main thread only
	);

	vkSJob->DependsOn(vkInJob.get());
	if (!wp->SubmitJob(std::move(vkInJob))) { throw std::runtime_error("Could not submit job\n"); }

	auto vkDvJob = wp->GetJob
	(
		[this](Jobs::WorkerPool* wp, size_t, Jobs::Job* thisJob)
		{
			vkDevice = std::make_unique<VKDevice>(wp, thisJob, this);
		},
		1,
		false
	);

	vkDvJob->DependsOn(vkSJob.get());
	if (!wp->SubmitJob(std::move(vkSJob))) { throw std::runtime_error("Could not submit job\n"); }

	using namespace std::placeholders;

	auto vkSCapsJob = wp->GetJob
	(
		[this](Jobs::WorkerPool*, size_t, Jobs::Job*)
		{
			this->vkSurface->UpdateCaps();
		},
		1,
		false
	);

	vkSCapsJob->DependsOn(vkDvJob.get());
	if (!wp->SubmitJob(std::move(vkDvJob))) { throw std::runtime_error("Could not submit job\n"); }

	auto vkSWJob = wp->GetJob
	(
		[this](Jobs::WorkerPool* wp, size_t, Jobs::Job* thisJob)
		{
			vkSwapchain = std::make_unique<VKSwapchain>(wp, thisJob, this);
		},
		1,
		false
	);

	vkSWJob->DependsOn(vkSCapsJob.get());
	if (!wp->SubmitJob(std::move(vkSCapsJob))) { throw std::runtime_error("Could not submit job\n"); }

	vkSWJob->SameThingsDependOnMeAs(thisJob);
	if (!wp->SubmitJob(std::move(vkSWJob))) { throw std::runtime_error("Could not submit job\n"); }
}

void CAM::Renderer::Renderer::DoFrame
(
	Jobs::WorkerPool* wp,
	size_t /*thread*/,
	Jobs::Job* thisJob
)
{
	/*
	 * [[M]window->HandleEvents] -> *
	 */

	using namespace std::placeholders;
	auto sdlJob = wp->GetJob
	(
		std::bind(&SDLWindow::HandleEvents, window.get(), _1, _2, _3),
		0,
		true // main thread only
	);

	sdlJob->SameThingsDependOnMeAs(thisJob);

	if (!wp->SubmitJob(std::move(sdlJob))) { throw std::runtime_error("Could not submit job\n"); }
}
bool CAM::Renderer::Renderer::ShouldContinue() { return window->ShouldContinue(); }

// NOTE: As acquire and as presenting possible
void CAM::Renderer::Renderer::SwapChainInvalidatedEvent(Jobs::Job* thisJob)
{
	/*
	 * [Resize Job Lambda] -> *
	 */

	auto resizeJob = wp->GetJob
	(
		[this](Jobs::WorkerPool*, size_t, Jobs::Job* thisJob)
		{
			vkSwapchain->RecreateSwapchain(thisJob);
		},
		0,
		false
	);

	resizeJob->SameThingsDependOnMeAs(thisJob);
	if (!wp->SubmitJob(std::move(resizeJob))) { throw std::runtime_error("Could not submit job\n"); }
}

