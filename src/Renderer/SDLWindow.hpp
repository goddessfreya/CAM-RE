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

#ifndef CAM_RENDERER_SDLWINDOW_HPP
#define CAM_RENDERER_SDLWINDOW_HPP

#include "../Jobs/Job.hpp"
#include "../Jobs/WorkerPool.hpp"

#include <cstdint>
#include <cstdio>

#include "Vulkan.h"
#include "SDL2/SDL.h"
#include "SDL2/SDL_syswm.h"
#include "SDL2/SDL_vulkan.h"

namespace CAM
{
namespace Renderer
{
class Renderer;

/*
 * Everthing in this class is callable from the main thread only unless
 * otherwise stated
 */
class SDLWindow
{
	public:
	SDLWindow(Jobs::WorkerPool* wp, Jobs::Job* thisJob, Renderer* parent);

	~SDLWindow();

	SDLWindow(const SDLWindow&) = default;
	SDLWindow(SDLWindow&&) = delete;
	SDLWindow& operator=(const SDLWindow&)& = default;
	SDLWindow& operator=(SDLWindow&&)& = delete;

	void HandleEvents
	(
		CAM::Jobs::WorkerPool* wp,
		size_t thread,
		CAM::Jobs::Job* thisJob
	);

	SDL_Window* operator()() { return window; }

	// Can be called from any threads
	[[nodiscard]] inline bool ShouldContinue() const
	{
		return shouldContinue.load(std::memory_order_acquire);
	}

	// Can be called from any threads
	[[nodiscard]] inline const std::vector<const char*>& GetReqExts() const { return reqExts; }

	private:
	CAM::Jobs::WorkerPool* UNUSED(wp);
	Renderer* UNUSED(parent);
	SDL_Window* window;

	std::atomic<bool> shouldContinue = true;

	std::vector<const char*> reqExts;
};
}
}

#endif
