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

#include "VKFNDevice.hpp"

namespace CAM
{
namespace Renderer
{
class Renderer;
class VKInstance;
class VKQueue;

struct DeviceData
{
	VkPhysicalDevice physicalDevice;
	VkPhysicalDeviceProperties physicalDeviceProperties;
	VkPhysicalDeviceFeatures physicalDeviceFeatures;

	std::vector<VkQueueFamilyProperties> queueFamilyProperties;

	struct ChosenQueues
	{
		inline bool FoundAll() const
		{
			return (!queueFamsWithGraphics.empty() || !dedicatedGraphicQueueFams.empty())
				// TODO: Enable compute if needed
				//&& (!queueFamsWithCompute.empty() || !dedicatedComputeQueueFams.empty())
				&& (!queueFamsWithTransfer.empty() || !dedicatedTransferQueueFams.empty())
				&& (!queueFamsWithSwap.empty() || !dedicatedSwapQueueFams.empty());
		}

		inline int IsDedicatedQueueFam(int count) const
		{
			auto& queueFam = queueFams[count];

			// First two are mising !transfer because they will always have transfer
			if (queueFam.graphics && !queueFam.swap && !queueFam.compute)
			{
				return 1;
			}
			else if (!queueFam.graphics && !queueFam.swap && queueFam.compute)
			{
				return 4; // 4 is intended
			}
			else if (queueFam.transfer && !queueFam.swap && !queueFam.compute && !queueFam.graphics)
			{
				return 2;
			}
			else if (queueFam.swap && !queueFam.compute && !queueFam.graphics && !queueFam.transfer)
			{
				return 3;
			}

			return 0;
		}

		inline int DedicatedQueueFams() const
		{
			int ret = 0;

			for (auto& qf : dedicatedGraphicQueueFams)
			{
				ret += queueFams[qf].count;
			}
			for (auto& qf : dedicatedTransferQueueFams)
			{
				ret += queueFams[qf].count;
			}
			for (auto& qf : dedicatedSwapQueueFams)
			{
				ret += queueFams[qf].count;
			}
			for (auto& qf : dedicatedComputeQueueFams)
			{
				ret += queueFams[qf].count;
			}

			return ret;
		}

		struct QueueFam
		{
			bool graphics = false;
			bool compute = false;
			bool transfer = false;
			bool swap = false;

			uint32_t count = 0;
		};

		std::vector<QueueFam> queueFams;

		// Won't include dedicated
		std::vector<int> queueFamsWithGraphics;
		std::vector<int> queueFamsWithTransfer;
		std::vector<int> queueFamsWithSwap;
		std::vector<int> queueFamsWithCompute;

		std::vector<int> dedicatedGraphicQueueFams;
		std::vector<int> dedicatedSwapQueueFams;
		std::vector<int> dedicatedTransferQueueFams;
		std::vector<int> dedicatedComputeQueueFams;

		int chosenGraphics;
		int chosenTransfer;
		int chosenSwap;

		bool chosenTransferIsGraphics = false;
		bool chosenSwapIsGraphics = false;
		bool chosenSwapIsTransfer = false;

		std::vector<std::unique_ptr<VKQueue>> queues;
	} queues;

	ChosenQueues ChooseQueues();

	VkDevice device;
	Renderer* parent;
};

// TODO: Maybe find the best Deivce Group for multi-gpu PCs
class VKDevice
{
	public:
	VKDevice(Jobs::WorkerPool* wp, Jobs::Job* thisJob, Renderer* parent);

	~VKDevice();

	VKDevice(const VKDevice&) = default;
	VKDevice(VKDevice&&) = delete;
	VKDevice& operator=(const VKDevice&)& = default;
	VKDevice& operator=(VKDevice&&)& = delete;

	inline VkDevice& operator()() { return devices[chosenDevice].device; }

	std::unique_ptr<DeviceVKFN> deviceVKFN;
	private:
	void PopulatePhysicalDeviceData();

	void RemoveAndSort();

	std::vector<const char*> GetDeviceExts();
	std::vector<VkDeviceQueueCreateInfo> GetDeviceQueues();

	void MakeDevice(uint32_t chosenDevice);

	static bool IsIncompatibleDevice(const DeviceData& a);
	static int RankDevice(const DeviceData& a);

	std::vector<DeviceData> devices;

	CAM::Jobs::WorkerPool* UNUSED(wp);
	Renderer* parent;
	VKInstance* vkInstance;

	uint32_t chosenDevice;

	const float priority = 1.f;

};
}
}

#endif
