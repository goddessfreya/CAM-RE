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

#include "VKDevice.hpp"
#include "Renderer.hpp"

CAM::Renderer::DeviceData::ChosenQueues CAM::Renderer::DeviceData::ChooseQueues() const
{
	ChosenQueues queues;
	queues.done = true;

	uint32_t i = 0;
	for(auto& queue : queueFamilyProperties)
	{
		if (queue.queueCount == 0)
		{
			++i;
			continue;
		}

		if ((queue.queueFlags & VK_QUEUE_COMPUTE_BIT) == VK_QUEUE_COMPUTE_BIT)
		{
			// Compute implies transfer
			if (!queues.dedicatedTransferQueue)
			{
				queues.foundTransferQueue = true;
				queues.transferQueue = i;
			}
		}

		if ((queue.queueFlags & VK_QUEUE_GRAPHICS_BIT) == VK_QUEUE_GRAPHICS_BIT)
		{
			// Graphics implies transfer
			if (!queues.dedicatedTransferQueue)
			{
				queues.foundTransferQueue = true;
				queues.transferQueue = i;
			}

			if (!queues.dedicatedGraphicsQueue)
			{
				queues.foundGraphicsQueue = true;
				queues.graphicsQueue = i;

				if ((queue.queueFlags & ~(VK_QUEUE_TRANSFER_BIT)) == VK_QUEUE_GRAPHICS_BIT)
				{
					queues.dedicatedGraphicsQueue = true;
				}
			}
		}

		if
		(
			(queue.queueFlags & VK_QUEUE_TRANSFER_BIT) == VK_QUEUE_TRANSFER_BIT
			&& !queues.dedicatedTransferQueue
		)
		{
			queues.foundTransferQueue = true;
			queues.transferQueue = i;

			if (queue.queueFlags == VK_QUEUE_TRANSFER_BIT)
			{
				queues.dedicatedTransferQueue = true;
			}
		}

		++i;
	}

	return queues;
}

CAM::Renderer::VKDevice::VKDevice
(
	Jobs::WorkerPool* wp,
	Jobs::Job* /*thisJob*/,
	Renderer* parent
) : wp(wp), parent(parent), vkInstance(parent->GetVKInstance())
{
	uint32_t devCount;
	VKFNCHECKRETURN(vkInstance->instanceVKFN->vkEnumeratePhysicalDevices
	(
		(*vkInstance)(),
		&devCount,
		nullptr
	));
	std::vector<VkPhysicalDevice> pds(devCount);
	VKFNCHECKRETURN(vkInstance->instanceVKFN->vkEnumeratePhysicalDevices
	(
		(*vkInstance)(),
		&devCount,
		pds.data()
	));

	for (auto& pd : pds)
	{
		devices.push_back({});
		devices.back().physicalDevice = std::move(pd);
	}

	PopulatePhysicalDeviceData();

	RemoveAndSort();
}

void CAM::Renderer::VKDevice::PopulatePhysicalDeviceData()
{
	printf("Physical Devices:\n");
	for (auto& device : devices)
	{
		vkInstance->instanceVKFN->vkGetPhysicalDeviceProperties
		(
			device.physicalDevice,
			&device.physicalDeviceProperties
		);

		printf("\t%s:\n", device.physicalDeviceProperties.deviceName);

		vkInstance->instanceVKFN->vkGetPhysicalDeviceFeatures
		(
			device.physicalDevice,
			&device.physicalDeviceFeatures
		);

		uint32_t queueSize;
		vkInstance->instanceVKFN->vkGetPhysicalDeviceQueueFamilyProperties
		(
			device.physicalDevice,
			&queueSize,
			nullptr
		);

		printf("\t\tQueue Families: %u\n", queueSize);

		device.queueFamilyProperties.resize(queueSize);
		vkInstance->instanceVKFN->vkGetPhysicalDeviceQueueFamilyProperties
		(
			device.physicalDevice,
			&queueSize,
			device.queueFamilyProperties.data()
		);

		printf("\t\tQueue Fam Props:\n");
		uint32_t i = 0;
		for (auto& queue : device.queueFamilyProperties)
		{
			printf("\t\t\tQueue Fam %u - %u Queue(s):\n", i, queue.queueCount);
			++i;

			if ((queue.queueFlags & VK_QUEUE_GRAPHICS_BIT) == VK_QUEUE_GRAPHICS_BIT)
			{
				printf("\t\t\t\tGraphics (Implies Transfer)\n");
			}

			if ((queue.queueFlags & VK_QUEUE_COMPUTE_BIT) == VK_QUEUE_COMPUTE_BIT)
			{
				printf("\t\t\t\tCompute (Implies Transfer)\n");
			}

			if ((queue.queueFlags & VK_QUEUE_TRANSFER_BIT) == VK_QUEUE_TRANSFER_BIT)
			{
				printf("\t\t\t\tTransfer\n");
			}

			if ((queue.queueFlags & VK_QUEUE_SPARSE_BINDING_BIT) == VK_QUEUE_SPARSE_BINDING_BIT)
			{
				printf("\t\t\t\tSparse\n");
			}

			if ((queue.queueFlags & VK_QUEUE_PROTECTED_BIT) == VK_QUEUE_PROTECTED_BIT)
			{
				printf("\t\t\t\tProtected\n");
			}
		}

		device.queues = device.ChooseQueues();

		printf("\t\tChosen Graphics Fam: %u\n", device.queues.graphicsQueue);
		printf("\t\tChosen Transfer Fam: %u\n", device.queues.transferQueue);
	}
}

void CAM::Renderer::VKDevice::RemoveAndSort()
{
	devices.erase(std::remove_if
	(
		std::begin(devices),
		std::end(devices),
		[] (const DeviceData& a)
		{
			return VKDevice::IsIncompatibleDevice(a);
		}
	), std::end(devices));

	std::sort
	(
		std::begin(devices),
		std::end(devices),
		[] (const DeviceData& a, const DeviceData& b)
		{
			return VKDevice::RankDevice(a) > VKDevice::RankDevice(b);
		}
	);
}

bool CAM::Renderer::VKDevice::IsIncompatibleDevice(const DeviceData& a)
{
	return !a.physicalDeviceFeatures.geometryShader && !a.queues.FoundAll();
}

int CAM::Renderer::VKDevice::RankDevice(const DeviceData& a)
{
	int rank = 0;

	if (a.physicalDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
	{
		rank += 1000;
	}

	rank += a.queues.DedicatedPools() * 100;

	return rank;
}

CAM::Renderer::VKDevice::~VKDevice()
{
}
