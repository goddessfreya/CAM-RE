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

#ifndef CAM_RENDERER_VKSEMAPHORE_HPP
#define CAM_RENDERER_VKSEMAPHORE_HPP

#include "../Jobs/Job.hpp"
#include "../Jobs/WorkerPool.hpp"

#include <cstdint>
#include <cstring>
#include <cstdio>

#include "Vulkan.h"
#include "SDL2/SDL.h"

namespace CAM
{
namespace Renderer
{
class VKInstance;

class VKSemaphore
{
	public:
	VKSemaphore(Jobs::WorkerPool* wp, Jobs::Job* thisJob, Renderer* parent);

	// All things using this semaphore must be completed before calling the deconstructor
	~VKSemaphore();

	VKSemaphore(const VKSemaphore&) = delete;
	VKSemaphore(VKSemaphore&&) = default;
	VKSemaphore& operator=(const VKSemaphore&)& = delete;
	VKSemaphore& operator=(VKSemaphore&&)& = default;

	inline std::pair<std::unique_lock<std::mutex>, VkSemaphore&> operator()()
	{
		return
		{
			std::unique_lock<std::mutex>(vkSemaphoreMutex, std::defer_lock),
			vkSemaphore
		};
	}

	private:

	CAM::Jobs::WorkerPool* UNUSED(wp);

	VkSemaphore vkSemaphore;
	mutable std::mutex vkSemaphoreMutex;

	Renderer* parent;
	VKInstance* UNUSED(instance);
};
}
}

#endif
