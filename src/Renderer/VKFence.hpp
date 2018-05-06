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

#ifndef CAM_RENDERER_VKFENCE_HPP
#define CAM_RENDERER_VKFENCE_HPP

#include "../Jobs/Job.hpp"
#include "../Jobs/WorkerPool.hpp"

#include <cstdint>
#include <cstring>
#include <cstdio>
#include <mutex>

#include "Vulkan.h"
#include "SDL2/SDL.h"

namespace CAM
{
namespace Renderer
{
class VKInstance;

class VKFence
{
	public:
	VKFence(Jobs::WorkerPool* wp, Jobs::Job* thisJob, Renderer* parent);

	// All things using this Fence must be completed before calling the deconstructor
	~VKFence();

	VKFence(const VKFence&) = delete;
	VKFence(VKFence&&) = default;
	VKFence& operator=(const VKFence&)& = delete;
	VKFence& operator=(VKFence&&)& = default;

	inline std::pair<std::unique_lock<std::mutex>, VkFence&> operator()()
	{
		return
		{
			std::unique_lock<std::mutex>(vkFenceMutex, std::defer_lock),
			vkFence
		};
	}
	bool IsReady();

	// TODO: Maybe implement a bulk-reset and bulk-waitfor if it realy matters
	void Reset();
	bool WaitFor(uint64_t timeout); // False if timed out

	private:
	CAM::Jobs::WorkerPool* UNUSED(wp);

	VkFence vkFence;
	mutable std::mutex vkFenceMutex;

	Renderer* parent;
	VKInstance* UNUSED(instance);
	VKDevice* device;

	int thisI;
};
}
}

#endif
