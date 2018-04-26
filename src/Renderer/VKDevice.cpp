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
#include "VKQueue.hpp"

CAM::Renderer::DeviceData::ChosenQueues CAM::Renderer::DeviceData::ChooseQueues()
{
	ChosenQueues queues;

	auto surface = parent->GetVKSurface();
	auto ins = parent->GetVKInstance();

	uint32_t i = 0;
	for(auto& queue : queueFamilyProperties)
	{
		queues.queueFams.push_back({});
		auto& thisQF = queues.queueFams.back();

		thisQF.count = queue.queueCount;

		if (queue.queueCount == 0)
		{
			++i;
			continue;
		}

		if ((queue.queueFlags & VK_QUEUE_COMPUTE_BIT) == VK_QUEUE_COMPUTE_BIT)
		{
			// Compute implies transfer
			thisQF.transfer = true;
			thisQF.compute = true;
		}

		if ((queue.queueFlags & VK_QUEUE_GRAPHICS_BIT) == VK_QUEUE_GRAPHICS_BIT)
		{
			// Graphics implies transfer
			thisQF.transfer = true;
			thisQF.graphics = true;
		}

		if ((queue.queueFlags & VK_QUEUE_TRANSFER_BIT) == VK_QUEUE_TRANSFER_BIT)
		{
			thisQF.transfer = true;
		}

		VkBool32 swapSupported;
		VKFNCHECKRETURN(ins->instanceVKFN->vkGetPhysicalDeviceSurfaceSupportKHR
		(
			physicalDevice,
			i,
			(*surface)(),
			&swapSupported
		));

		if (swapSupported == VK_TRUE)
		{
			thisQF.swap = true;
		}

		printf("\t\t QueueFam %i - %i Queues\n", i, thisQF.count);
		switch(queues.IsDedicatedQueueFam(i))
		{
			case 1:
				queues.dedicatedGraphicQueueFams.push_back(i);
				printf("\t\t\tDedicated Graphics (implies transfer)\n");
				break;
			case 2:
				queues.dedicatedTransferQueueFams.push_back(i);
				printf("\t\t\tDedicated Transfer (implies transfer)\n");
				break;
			case 3:
				queues.dedicatedSwapQueueFams.push_back(i);
				printf("\t\t\tDedicated Swap\n");
				break;
			case 4:
				queues.dedicatedComputeQueueFams.push_back(i);
				printf("\t\t\tDedicated Compute\n");
				break;
			default:
				if (thisQF.graphics)
				{
					queues.queueFamsWithGraphics.push_back(i);
					printf("\t\t\tGraphics\n");
				}

				if (thisQF.transfer)
				{
					queues.queueFamsWithTransfer.push_back(i);
					printf("\t\t\tTransfer\n");
				}

				if (thisQF.swap)
				{
					queues.queueFamsWithSwap.push_back(i);
					printf("\t\t\tSwap\n");
				}

				if (thisQF.compute)
				{
					queues.queueFamsWithCompute.push_back(i);
					printf("\t\t\tCompute\n");
				}
				break;
		}
		++i;
	}

	auto sortFunc = [&queues] (const int& a, const int& b)
	{
		auto& aQ = queues.queueFams[a];
		auto& bQ = queues.queueFams[b];
		return aQ.count > bQ.count;
	};
	std::sort(std::begin(queues.dedicatedGraphicQueueFams), std::end(queues.dedicatedGraphicQueueFams), sortFunc);
	std::sort(std::begin(queues.dedicatedComputeQueueFams), std::end(queues.dedicatedComputeQueueFams), sortFunc);
	std::sort(std::begin(queues.dedicatedTransferQueueFams), std::end(queues.dedicatedTransferQueueFams), sortFunc);
	std::sort(std::begin(queues.dedicatedSwapQueueFams), std::end(queues.dedicatedSwapQueueFams), sortFunc);
	std::sort(std::begin(queues.queueFamsWithGraphics), std::end(queues.queueFamsWithGraphics), sortFunc);
	std::sort(std::begin(queues.queueFamsWithCompute), std::end(queues.queueFamsWithCompute), sortFunc);
	std::sort(std::begin(queues.queueFamsWithTransfer), std::end(queues.queueFamsWithTransfer), sortFunc);
	std::sort(std::begin(queues.queueFamsWithSwap), std::end(queues.queueFamsWithSwap), sortFunc);

	return queues;
}

CAM::Renderer::VKDevice::VKDevice
(
	Jobs::WorkerPool* wp,
	Jobs::Job* /*thisJob*/,
	Renderer* parent
) : wp(wp), parent(parent), vkInstance(this->parent->GetVKInstance())
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
	MakeDevice(0);
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

		device.queueFamilyProperties.resize(queueSize);
		vkInstance->instanceVKFN->vkGetPhysicalDeviceQueueFamilyProperties
		(
			device.physicalDevice,
			&queueSize,
			device.queueFamilyProperties.data()
		);

		device.parent = parent;
		device.queues = device.ChooseQueues();
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

	if (devices.empty())
	{
		throw std::runtime_error("No devices are compatible");
	}

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
	return !a.physicalDeviceFeatures.geometryShader || !a.queues.FoundAll();
}

int CAM::Renderer::VKDevice::RankDevice(const DeviceData& a)
{
	int rank = 0;

	if (a.physicalDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
	{
		rank += 1000;
	}

	rank += a.queues.DedicatedQueueFams() * 10;

	return rank;
}

std::vector<VkDeviceQueueCreateInfo> CAM::Renderer::VKDevice::GetDeviceQueues()
{
	auto& device = devices[chosenDevice];
	std::vector<VkDeviceQueueCreateInfo> ret;

	if (!device.queues.dedicatedGraphicQueueFams.empty())
	{
		device.queues.chosenGraphics = device.queues.dedicatedGraphicQueueFams.front();
	}
	else
	{
		device.queues.chosenGraphics = device.queues.queueFamsWithGraphics.front();
	}

	if (!device.queues.dedicatedTransferQueueFams.empty())
	{
		device.queues.chosenTransfer = device.queues.dedicatedTransferQueueFams.front();
	}
	else
	{
		device.queues.chosenTransfer = device.queues.queueFamsWithTransfer.front();
		if (device.queues.chosenTransfer == device.queues.chosenGraphics)
		{
			for (auto& qU : device.queues.queueFamsWithTransfer)
			{
				device.queues.chosenTransfer = qU;

				if (device.queues.chosenTransfer != device.queues.chosenGraphics)
				{
					break;
				}
			}

			if (device.queues.chosenTransfer == device.queues.chosenGraphics)
			{
				if (device.queues.queueFams[device.queues.chosenTransfer].count == 1)
				{
					device.queues.chosenTransferIsGraphics = true;
				}
			}
		}
	}

	if (!device.queues.dedicatedSwapQueueFams.empty())
	{
		device.queues.chosenSwap = device.queues.dedicatedSwapQueueFams.front();
	}
	else
	{
		device.queues.chosenSwap = device.queues.queueFamsWithTransfer.front();
		if (device.queues.chosenSwap == device.queues.chosenGraphics
			|| device.queues.chosenSwap == device.queues.chosenTransfer)
		{
			for (auto& qU : device.queues.queueFamsWithTransfer)
			{
				device.queues.chosenSwap = qU;

				if (device.queues.chosenSwap != device.queues.chosenGraphics
					&& device.queues.chosenSwap != device.queues.chosenTransfer)
				{
					break;
				}
			}

			if (device.queues.chosenSwap == device.queues.chosenGraphics
				&& device.queues.chosenSwap == device.queues.chosenTransfer)
			{
				if (device.queues.queueFams[device.queues.chosenSwap].count < 3)
				{
					device.queues.chosenSwapIsGraphics = true;
					device.queues.chosenSwapIsTransfer = true;
				}
			}
			else if (device.queues.chosenSwap == device.queues.chosenGraphics
				|| device.queues.chosenSwap == device.queues.chosenTransfer)
			{
				if (device.queues.queueFams[device.queues.chosenSwap].count < 2)
				{
					if (device.queues.chosenSwap == device.queues.chosenGraphics)
					{
						device.queues.chosenSwapIsGraphics = true;
					}
					else
					{
						device.queues.chosenSwapIsTransfer = true;
					}
				}
			}
		}
	}

	VkDeviceQueueCreateInfo current;
	current.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	current.pNext = nullptr;
	current.flags = 0;
	current.pQueuePriorities = &priority;

	current.queueFamilyIndex = device.queues.chosenGraphics;
	int queues = 1;
	if (device.queues.chosenTransfer == device.queues.chosenGraphics)
	{
		if (!device.queues.chosenTransferIsGraphics)
		{
			++queues;
		}
	}
	if (device.queues.chosenSwap == device.queues.chosenGraphics)
	{
		if (!device.queues.chosenSwapIsGraphics)
		{
			++queues;
		}
	}
	current.queueCount = queues;

	ret.push_back(current);

	if (device.queues.chosenTransfer != device.queues.chosenGraphics)
	{
		current.queueFamilyIndex = device.queues.chosenTransfer;
		queues = 1;

		if (device.queues.chosenTransfer == device.queues.chosenSwap)
		{
			if (!device.queues.chosenSwapIsTransfer)
			{
				++queues;
			}
		}
		current.queueCount = queues;
		ret.push_back(current);
	}

	if (device.queues.chosenSwap != device.queues.chosenGraphics && device.queues.chosenSwap != device.queues.chosenTransfer)
	{
		current.queueFamilyIndex = device.queues.chosenSwap;
		current.queueCount = 1;
		ret.push_back(current);
	}

	return ret;
}

std::vector<const char*> CAM::Renderer::VKDevice::GetDeviceExts()
{
	auto& device = devices[chosenDevice];

	uint32_t count;
	VKFNCHECKRETURN(vkInstance->instanceVKFN->vkEnumerateDeviceExtensionProperties
	(
		device.physicalDevice, nullptr, &count, nullptr
	));
	std::vector<VkExtensionProperties> exts(count);
	VKFNCHECKRETURN(vkInstance->instanceVKFN->vkEnumerateDeviceExtensionProperties
	(
		device.physicalDevice, nullptr, &count, exts.data()
	));

	std::vector<const char*> rets =
	{
		"VK_KHR_swapchain"
	};

	printf("\tAvailable exts:\n");
	for(auto& ext : exts)
	{
		printf("\t\t%s\n", ext.extensionName);
	}

	printf("\tRequested Exts:\n");
	for (auto& ret : rets)
	{
		printf("\t\t%s\n", ret);
		bool found = false;
		for (auto& aExt : exts)
		{
			if (std::strcmp(aExt.extensionName, ret))
			{
				found = true;
				break;
			}
		}
		if (!found)
		{
			throw std::runtime_error("Couldn't find ext " + std::string(ret));
		}
	}

	return rets;
}

void CAM::Renderer::VKDevice::MakeDevice(uint32_t chosenDevice)
{
	this->chosenDevice = chosenDevice;
	auto& device = devices[chosenDevice];

	printf("Chosen device: %s\n", device.physicalDeviceProperties.deviceName);

	VkDeviceCreateInfo createInfo;
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.flags = 0;
	auto queues = GetDeviceQueues();
	createInfo.queueCreateInfoCount = queues.size();
	createInfo.pQueueCreateInfos = queues.data();
	createInfo.enabledLayerCount = 0;
	createInfo.ppEnabledLayerNames = nullptr;
	auto exts = GetDeviceExts();
	createInfo.enabledExtensionCount = exts.size();
	createInfo.ppEnabledExtensionNames = exts.data();

	VkPhysicalDeviceFeatures features = {};
	features.geometryShader = VK_TRUE;
	createInfo.pEnabledFeatures = &features;

	VKFNCHECKRETURN(vkInstance->instanceVKFN->vkCreateDevice(device.physicalDevice, &createInfo, nullptr, &device.device));

	deviceVKFN = std::make_unique<DeviceVKFN>(&operator()(), vkInstance->instanceVKFN->vkGetDeviceProcAddr);
	if (!deviceVKFN->InitDeviceFuncs()) { throw std::runtime_error("Could not init instance funcs\n"); }
}

CAM::Renderer::VKDevice::~VKDevice()
{
	VKFNCHECKRETURN(deviceVKFN->vkDeviceWaitIdle(operator()()));
	vkInstance->instanceVKFN->vkDestroyDevice(operator()(), nullptr);
}
