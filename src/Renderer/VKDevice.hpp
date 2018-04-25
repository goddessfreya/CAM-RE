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

#ifndef CAM_RENDERER_VKDEVICE_HPP
#define CAM_RENDERER_VKDEVICE_HPP

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
class Renderer;
class VKInstance;

struct DeviceData
{
	VkPhysicalDevice physicalDevice;
	VkPhysicalDeviceProperties physicalDeviceProperties;
	VkPhysicalDeviceFeatures physicalDeviceFeatures;

	std::vector<VkQueueFamilyProperties> queueFamilyProperties;

	struct ChosenQueues
	{
		bool done = false;

		inline bool FoundAll() const
		{
			return foundGraphicsQueue && foundTransferQueue;
		}

		inline int DedicatedPools() const
		{
			int ret = 0;
			if (dedicatedGraphicsQueue && foundGraphicsQueue)
			{
				++ret;
			}

			if (dedicatedTransferQueue && foundTransferQueue)
			{
				++ret;
			}

			return ret;
		}

		bool foundGraphicsQueue = false;
		bool dedicatedGraphicsQueue = false;
		uint32_t graphicsQueue;

		bool foundTransferQueue = false;
		bool dedicatedTransferQueue = false;
		uint32_t transferQueue;

		// TODO: Add later
		//bool foundPresentQueue = false;
		//bool dedicatedPresentQueue = false;
		//uint32_t presentQueue;
	} queues;

	int NumberOfDedicatedQueues() const;
	ChosenQueues ChooseQueues() const;
};

class VKDevice
{
	public:
	VKDevice(Jobs::WorkerPool* wp, Jobs::Job* thisJob, Renderer* parent);

	~VKDevice();

	VKDevice(const VKDevice&) = default;
	VKDevice(VKDevice&&) = delete;
	VKDevice& operator=(const VKDevice&)& = default;
	VKDevice& operator=(VKDevice&&)& = delete;

	private:
	void PopulatePhysicalDeviceData();

	void RemoveAndSort();

	static bool IsIncompatibleDevice(const DeviceData& a);
	static int RankDevice(const DeviceData& a);

	std::vector<DeviceData> devices;

	CAM::Jobs::WorkerPool* UNUSED(wp);
	Renderer* UNUSED(parent);
	VKInstance* vkInstance;
};
}
}

#endif
