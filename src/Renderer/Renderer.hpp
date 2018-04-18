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

#ifndef CAM_RENDERER_RENDERER_HPP
#define CAM_RENDERER_RENDERER_HPP

#include "../Jobs/Job.hpp"
#include "../Jobs/WorkerPool.hpp"

#include <cstdint>
#include <cstdio>
#include <cassert>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace CAM
{
namespace Renderer
{
class Renderer
{
	public:
	Renderer(Jobs::WorkerPool* wp, Jobs::Job* thisJob);

	void DoFrame
	(
		void* userData,
		CAM::Jobs::WorkerPool* wp,
		size_t thread,
		CAM::Jobs::Job* thisJob
	);
	bool ShouldContinue();

	private:
};
}
}

#endif
