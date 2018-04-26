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

#ifndef CAM_RENDERER_VKQUEUE_HPP
#define CAM_RENDERER_VKQUEUE_HPP

#include "../Jobs/Job.hpp"
#include "../Jobs/WorkerPool.hpp"

#include <cstdint>
#include <cstring>
#include <cstdio>

#include "Vulkan.h"
#include "SDL2/SDL.h"

#include "VKFNDevice.hpp"

namespace CAM
{
namespace Renderer
{
class VKQueue
{
	public:
	VKQueue(Jobs::WorkerPool* wp, VKDevice* parent);
	~VKQueue();

	VKQueue(const VKQueue&) = default;
	VKQueue(VKQueue&&) = delete;
	VKQueue& operator=(const VKQueue&)& = default;
	VKQueue& operator=(VKQueue&&)& = delete;

	inline VkQueue& operator()() { return queue; }

	CAM::Jobs::WorkerPool* UNUSED(wp);
	VKDevice* UNUSED(parent);
	VkQueue queue;
};
}
}

#endif
