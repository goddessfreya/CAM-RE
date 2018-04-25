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
	 * [[M]SDLWindow Lambda] --\
	 * [InitGlobalFuncs Lambda] => [VKInstance Lambda] -> [VKDevice Lambda] -> *
	 */

	auto sdlJob = wp->GetJob
	(
		[this](void*, Jobs::WorkerPool* wp, size_t, Jobs::Job* thisJob)
		{
			window = std::make_unique<SDLWindow>(wp, thisJob, this);
		},
		nullptr,
		1,
		true // main thread only
	);

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
	vkInJob->DependsOn(sdlJob.get());
	if (!wp->SubmitJob(std::move(igFNJob))) { throw std::runtime_error("Could not submit job\n"); }
	if (!wp->SubmitJob(std::move(sdlJob))) { throw std::runtime_error("Could not submit job\n"); }

	auto vkDvJob = wp->GetJob
	(
		[this](void*, Jobs::WorkerPool* wp, size_t, Jobs::Job* thisJob)
		{
			vkDevice = std::make_unique<VKDevice>(wp, thisJob, this);
		},
		nullptr,
		1,
		false
	);

	vkDvJob->DependsOn(vkInJob.get());
	if (!wp->SubmitJob(std::move(vkInJob))) { throw std::runtime_error("Could not submit job\n"); }

	vkDvJob->SameThingsDependOnMeAs(thisJob);
	if (!wp->SubmitJob(std::move(vkDvJob))) { throw std::runtime_error("Could not submit job\n"); }
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

	using namespace std::placeholders;
	auto sdlJob = wp->GetJob
	(
		std::bind(&SDLWindow::HandleEvents, window.get(), _1, _2, _3, _4),
		nullptr,
		0,
		true // main thread only
	);

	sdlJob->SameThingsDependOnMeAs(thisJob);

	if (!wp->SubmitJob(std::move(sdlJob))) { throw std::runtime_error("Could not submit job\n"); }
}
bool CAM::Renderer::Renderer::ShouldContinue() { return window->ShouldContinue(); }
