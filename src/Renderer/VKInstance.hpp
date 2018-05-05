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
#include <cstring>
#include <cstdio>

#include "Vulkan.h"
#include "SDL2/SDL.h"

#include "VKFNGlobal.hpp"
#include "VKFNInstance.hpp"
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

	VKInstance(const VKInstance&) = default;
	VKInstance(VKInstance&&) = delete;
	VKInstance& operator=(const VKInstance&)& = default;
	VKInstance& operator=(VKInstance&&)& = delete;

	inline VkInstance& operator()() { return instance; }

	std::unique_ptr<InstanceVKFN> instanceVKFN;

	private:
	static VkBool32 VKAPI_PTR DebugCallback
	(
		VkDebugReportFlagsEXT flags,
		VkDebugReportObjectTypeEXT objectType,
		uint64_t object,
		size_t location,
		int32_t messageCode,
		const char* pLayerPrefix,
		const char* pMessage,
		void* pUserData
	);

	std::vector<const char*> layers;
	std::vector<const char*> exts;

	CAM::Jobs::WorkerPool* UNUSED(wp);
	Renderer* UNUSED(parent);

	VkInstance instance;

	VkDebugReportCallbackEXT dcb;
};
}
}

#endif
