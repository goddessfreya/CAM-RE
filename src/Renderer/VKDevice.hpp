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

enum QueueType
{
	Graphics,
	Transfer,
	Swap
};

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
			return (!queueFamsWithGraphics.empty() || !perferredGraphicQueueFams.empty())
				// TODO: Enable compute if needed
				//&& (!queueFamsWithCompute.empty() || !perferredComputeQueueFams.empty())
				&& (!queueFamsWithTransfer.empty() || !perferredTransferQueueFams.empty())
				&& (!queueFamsWithSwap.empty() || !perferredSwapQueueFams.empty());
		}

		inline int IsPerferredQueueFam(int count) const
		{
			auto& queueFam = queueFams[count];
			auto ret = 0;

			// bit 1 = graphics
			// bit 2 = compute
			// bit 3 = transfer
			// bit 4 = swap

			if (queueFam.graphics && !queueFam.compute)
			{
				ret |= 1;
			}
			else if (!queueFam.graphics && queueFam.compute)
			{
				ret |= 2;
			}
			else if (queueFam.transfer && !queueFam.swap && !queueFam.compute && !queueFam.graphics)
			{
				ret |= 4;
			}
			else if (queueFam.swap && queueFam.graphics)
			{
				ret |= 8;
			}

			return ret;
		}

		inline int PerferredQueueFams() const
		{
			int ret = 0;

			for (auto& qf : perferredGraphicQueueFams)
			{
				ret += queueFams[qf].count;
			}
			for (auto& qf : perferredTransferQueueFams)
			{
				ret += queueFams[qf].count;
			}
			for (auto& qf : perferredSwapQueueFams)
			{
				ret += queueFams[qf].count;
			}
			for (auto& qf : perferredComputeQueueFams)
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

		// Won't include matching perferred
		std::vector<uint32_t> queueFamsWithGraphics;
		std::vector<uint32_t> queueFamsWithTransfer;
		std::vector<uint32_t> queueFamsWithSwap;
		std::vector<uint32_t> queueFamsWithCompute;

		std::vector<uint32_t> perferredGraphicQueueFams;
		std::vector<uint32_t> perferredSwapQueueFams; // We perfer if swap is shared with graphics
		std::vector<uint32_t> perferredTransferQueueFams;
		std::vector<uint32_t> perferredComputeQueueFams;

		int chosenGraphics;
		int chosenTransfer;
		int chosenSwap;

		bool chosenTransferIsGraphics = false;
		bool chosenSwapIsGraphics = false;
		bool chosenSwapIsTransfer = false;

		std::vector<std::unique_ptr<VKQueue>> queues;

		VKQueue* graphics;
		VKQueue* transfer;
		VKQueue* swap;
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
	inline VkPhysicalDevice& GetPhysicalDevice() { return devices[chosenDevice].physicalDevice; }

	std::unique_ptr<DeviceVKFN> deviceVKFN;

	inline VKQueue* GetQueue(QueueType qt)
	{
		auto& queues = devices[chosenDevice].queues;
		switch (qt)
		{
			case QueueType::Graphics:
				return queues.graphics;
			case QueueType::Transfer:
				return queues.transfer;
			case QueueType::Swap:
				return queues.swap;
		}
	}

	private:
	void PopulatePhysicalDeviceData();

	void RemoveAndSort();

	std::vector<const char*> GetDeviceExts();
	std::vector<VkDeviceQueueCreateInfo> GetDeviceQueues();

	void MakeDevice(uint32_t chosenDevice);
	void MakeQueues();

	static bool IsIncompatibleDevice(const DeviceData& a);
	static int RankDevice(const DeviceData& a);

	std::vector<DeviceData> devices;

	CAM::Jobs::WorkerPool* wp;
	Renderer* parent;
	VKInstance* vkInstance;

	uint32_t chosenDevice;

	const float priority = 1.f;

};
}
}

#endif
