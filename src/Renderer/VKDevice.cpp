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

		VkBool32 presentSupported;
		VKFNCHECKRETURN(ins->instanceVKFN->vkGetPhysicalDeviceSurfaceSupportKHR
		(
			physicalDevice,
			i,
			(*surface)(),
			&presentSupported
		));

		if (presentSupported == VK_TRUE)
		{
			thisQF.present = true;
		}

		printf("\t\t QueueFam %i - %i Queues\n", i, thisQF.count);
		auto p = queues.IsPerferredQueueFam(i);

		if ((p & 1) == 1)
		{
			queues.perferredGraphicQueueFams.push_back(i);
			printf("\t\t\tPerferred Graphics\n");
		}

		if ((p & 2) == 2)
		{
			queues.perferredComputeQueueFams.push_back(i);
			printf("\t\t\tPerferred Compute\n");
		}

		if ((p & 4) == 4)
		{
			queues.perferredTransferQueueFams.push_back(i);
			printf("\t\t\tPerferred Transfer\n");
		}

		if ((p & 8) == 8)
		{
			queues.perferredPresentQueueFams.push_back(i);
			printf("\t\t\tPerfered Present\n");
		}

		if (thisQF.graphics && (queues.perferredGraphicQueueFams.empty() || i != queues.perferredGraphicQueueFams.back()))
		{
			queues.queueFamsWithGraphics.push_back(i);
			printf("\t\t\tGraphics\n");
		}

		if (thisQF.transfer && (queues.perferredTransferQueueFams.empty() || i != queues.perferredTransferQueueFams.back()))
		{
			queues.queueFamsWithTransfer.push_back(i);
			printf("\t\t\tTransfer\n");
		}

		if (thisQF.compute && (queues.perferredComputeQueueFams.empty() || i != queues.perferredComputeQueueFams.back()))
		{
			queues.queueFamsWithCompute.push_back(i);
			printf("\t\t\tCompute\n");
		}

		if (thisQF.present && (queues.perferredPresentQueueFams.empty() || i != queues.perferredPresentQueueFams.back()))
		{
			queues.queueFamsWithPresent.push_back(i);
			printf("\t\t\tPresent\n");
		}

		++i;
	}

	auto sortFunc = [&queues] (const int& a, const int& b)
	{
		auto& aQ = queues.queueFams[a];
		auto& bQ = queues.queueFams[b];
		return aQ.count > bQ.count;
	};
	std::sort(std::begin(queues.perferredGraphicQueueFams), std::end(queues.perferredGraphicQueueFams), sortFunc);
	std::sort(std::begin(queues.perferredComputeQueueFams), std::end(queues.perferredComputeQueueFams), sortFunc);
	std::sort(std::begin(queues.perferredTransferQueueFams), std::end(queues.perferredTransferQueueFams), sortFunc);
	std::sort(std::begin(queues.perferredPresentQueueFams), std::end(queues.perferredPresentQueueFams), sortFunc);
	std::sort(std::begin(queues.queueFamsWithGraphics), std::end(queues.queueFamsWithGraphics), sortFunc);
	std::sort(std::begin(queues.queueFamsWithCompute), std::end(queues.queueFamsWithCompute), sortFunc);
	std::sort(std::begin(queues.queueFamsWithTransfer), std::end(queues.queueFamsWithTransfer), sortFunc);
	std::sort(std::begin(queues.queueFamsWithPresent), std::end(queues.queueFamsWithPresent), sortFunc);

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
	MakeQueues();
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

	rank += a.queues.PerferredQueueFams() * 10;

	return rank;
}

// We will perfer perferred queues, if that doesn't work out we will try the shared ones
std::vector<VkDeviceQueueCreateInfo> CAM::Renderer::VKDevice::GetDeviceQueues()
{
	auto& device = devices[chosenDevice];
	std::vector<VkDeviceQueueCreateInfo> ret;

	if (!device.queues.perferredGraphicQueueFams.empty())
	{
		device.queues.chosenGraphics = device.queues.perferredGraphicQueueFams.front();
	}
	else
	{
		// no possible collision
		device.queues.chosenGraphics = device.queues.queueFamsWithGraphics.front();
	}

	bool doNext = false;
	if (!device.queues.perferredTransferQueueFams.empty())
	{
		for (auto& qU : device.queues.perferredTransferQueueFams)
		{
			device.queues.chosenTransfer = qU;

			if
			(
				device.queues.chosenTransfer != device.queues.chosenGraphics
				|| device.queues.queueFams[device.queues.chosenTransfer].count > 1
			)
			{
				break;
			}
		}
	}
	else
	{
		doNext = true;
	}

	if
	(
		doNext
		||
		(
			device.queues.chosenTransfer == device.queues.chosenGraphics
			&& device.queues.queueFams[device.queues.chosenTransfer].count == 1
		)
	)
	{
		for (auto& qU : device.queues.queueFamsWithTransfer)
		{
			device.queues.chosenTransfer = qU;

			if
			(
				device.queues.chosenTransfer != device.queues.chosenGraphics
				|| device.queues.queueFams[device.queues.chosenTransfer].count > 1
			)
			{
				break;
			}
		}
	}

	if
	(
		device.queues.chosenTransfer == device.queues.chosenGraphics
		&& device.queues.queueFams[device.queues.chosenTransfer].count == 1
	)
	{
		device.queues.chosenTransferIsGraphics = true;
	}

	doNext = false;
	if (!device.queues.perferredPresentQueueFams.empty())
	{
		for (auto& qU : device.queues.perferredPresentQueueFams)
		{
			device.queues.chosenPresent = qU;

			if(device.queues.chosenPresent == device.queues.chosenGraphics)
			{
				device.queues.chosenPresentIsGraphics = true;
				device.queues.chosenPresentIsTransfer = device.queues.chosenTransferIsGraphics;
				break;
			}
		}
	}
	else
	{
		doNext = true;
	}

	if (doNext || device.queues.chosenPresent != device.queues.chosenGraphics)
	{
		for (auto& qU : device.queues.queueFamsWithTransfer)
		{
			device.queues.chosenTransfer = qU;

			if
			(
				device.queues.chosenPresent != device.queues.chosenTransfer
				&& device.queues.queueFams[device.queues.chosenTransfer].count == 1
			)
			{
				device.queues.chosenTransferIsGraphics = true;
				break;
			}
		}
	}

	if
	(
		device.queues.chosenPresent != device.queues.chosenGraphics
		&& device.queues.chosenPresent == device.queues.chosenTransfer
		&& device.queues.queueFams[device.queues.chosenTransfer].count == 1
	)
	{
		device.queues.chosenTransferIsGraphics = true;
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
	if (device.queues.chosenPresent == device.queues.chosenGraphics)
	{
		if (!device.queues.chosenPresentIsGraphics)
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

		if (device.queues.chosenTransfer == device.queues.chosenPresent)
		{
			if (!device.queues.chosenPresentIsTransfer)
			{
				++queues;
			}
		}
		current.queueCount = queues;
		ret.push_back(current);
	}

	if (device.queues.chosenPresent != device.queues.chosenGraphics && device.queues.chosenPresent != device.queues.chosenTransfer)
	{
		current.queueFamilyIndex = device.queues.chosenPresent;
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

void CAM::Renderer::VKDevice::MakeQueues()
{
	auto& device = devices[chosenDevice];

	printf("Graphics: ");
	device.queues.queues.push_back(std::make_unique<VKQueue>
	(
		wp,
		this,
		device.queues.chosenGraphics,
		0
	));

	device.queues.graphics = device.queues.queues.back().get();

	int index = -1;

	if (device.queues.chosenTransfer == device.queues.chosenGraphics)
	{
		if (!device.queues.chosenTransferIsGraphics)
		{
			index = 1;
		}
	}
	else
	{
		index = 0;
	}

	if (index != -1)
	{
		printf("Transfer: ");
		device.queues.queues.push_back(std::make_unique<VKQueue>
		(
			wp,
			this,
			device.queues.chosenTransfer,
			index
		));
	}
	else
	{
		printf("Transfer is graphics\n");
	}

	device.queues.transfer = device.queues.queues.back().get();

	if (device.queues.chosenPresentIsGraphics || device.queues.chosenPresentIsTransfer)
	{
		index = -1;
		if (device.queues.chosenPresentIsGraphics)
		{
			printf("Present is graphics\n");
			device.queues.present = device.queues.graphics;
		}
		else
		{
			printf("Present is transfer\n");
			device.queues.present = device.queues.transfer;
		}
	}
	else if
	(
		device.queues.chosenPresent == device.queues.chosenTransfer
		&& device.queues.chosenPresent == device.queues.chosenGraphics
	)
	{
		index = 2;
	}
	else if
	(
		device.queues.chosenPresent == device.queues.chosenTransfer
		|| device.queues.chosenPresent == device.queues.chosenGraphics
	)
	{
		index = 1;
	}
	else
	{
		index = 0;
	}

	if (index != -1)
	{
		printf("Present: ");
		device.queues.queues.push_back(std::make_unique<VKQueue>
		(
			wp,
			this,
			device.queues.chosenTransfer,
			index
		));
		device.queues.present = device.queues.queues.back().get();
	}
}
