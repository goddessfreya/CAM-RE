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
#include <cassert>

#include "SDL2/SDL.h"
#include <vulkan/vulkan.h>

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

	SDLWindow(const SDLWindow&) = delete;
	SDLWindow(SDLWindow&&) = default;
	SDLWindow& operator=(const SDLWindow&)& = delete;
	SDLWindow& operator=(SDLWindow&&)& = default;

	void HandleEvents
	(
		void* userData,
		CAM::Jobs::WorkerPool* wp,
		size_t thread,
		CAM::Jobs::Job* thisJob
	);

	// Can be called from any threads
	[[nodiscard]] inline bool ShouldContinue() const { return shouldContinue; }

	// Can be called from any threads
	[[nodiscard]] inline std::pair<int, int> GetSize() const { return {width, height}; }

	private:
	CAM::Jobs::WorkerPool* UNUSED(wp);
	Renderer* UNUSED(parent);
	SDL_Window* window;

	bool shouldContinue = true;

	int width;
	int height;
};
}
}

#endif
