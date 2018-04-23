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

#ifndef CAM_RENDERER_VKINSTANCE_HPP
#define CAM_RENDERER_VKINSTANCE_HPP

#include "../Jobs/Job.hpp"
#include "../Jobs/WorkerPool.hpp"

#include <cstdint>
#include <cstdio>
#include <cassert>

#include "SDL2/SDL.h"
#include <vulkan/vulkan.h>

#include "VKFNGlobal.hpp"
#include "VKCheckReturn.hpp"

namespace CAM
{
namespace Renderer
{
class Renderer;

class VKInstance
{
	public:
	VKInstance(Jobs::WorkerPool* wp, Jobs::Job* thisJob, Renderer* parent);

	~VKInstance();

	VKInstance(const VKInstance&) = delete;
	VKInstance(VKInstance&&) = default;
	VKInstance& operator=(const VKInstance&)& = delete;
	VKInstance& operator=(VKInstance&&)& = default;

	private:
	CAM::Jobs::WorkerPool* UNUSED(wp);
	Renderer* UNUSED(parent);
	VkInstance* UNUSED(instance);
};
}
}

#endif
