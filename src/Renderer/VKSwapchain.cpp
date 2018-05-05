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

#include "SDLWindow.hpp"
#include "Renderer.hpp"
#include "VKSwapchain.hpp"
#include "../Config.hpp"

CAM::Renderer::VKSwapchain::VKSwapchain(Jobs::WorkerPool* wp, Jobs::Job* thisJob, Renderer* parent)
	: wp(wp),
	parent(parent),
	device(parent->GetVKDevice()),
	surface(parent->GetVKSurface()),
	instance(parent->GetVKInstance()),
	window(parent->GetSDLWindow())
{
	RecreateSwapchain(thisJob);
}

void CAM::Renderer::VKSwapchain::RecreateSwapchain(Jobs::Job* thisJob)
{
	/*
	 * [[M]Get Window Size lambda] -> *
	 */
	auto getWinSizeJob = wp->GetJob
	(
		[this](Jobs::WorkerPool* wp, size_t, Jobs::Job* thisJob)
		{
			int width;
			int height;

			SDL_GetWindowSize((*window)(), &width, &height);

			/*
			 * [RecreateSwapchain_Internal] -> *
			 */
			auto rchJob = wp->GetJob
			(
				[this, width, height](Jobs::WorkerPool*, size_t, Jobs::Job*)
				{
					RecreateSwapchain_Internal((uint32_t)width, (uint32_t)height);
				},
				0,
				false
			);

			rchJob->SameThingsDependOnMeAs(thisJob);
			if (!wp->SubmitJob(std::move(rchJob))) { throw std::runtime_error("Could not submit job\n"); }
		},
		0,
		true // main-thread only
	);

	getWinSizeJob->SameThingsDependOnMeAs(thisJob);
	if (!wp->SubmitJob(std::move(getWinSizeJob))) { throw std::runtime_error("Could not submit job\n"); }
}

void CAM::Renderer::VKSwapchain::RecreateSwapchain_Internal(uint32_t width, uint32_t height)
{
	vkOldSwapchain = VK_NULL_HANDLE;
	std::swap(vkSwapchain, vkOldSwapchain);

	VkSwapchainCreateInfoKHR createInfo;
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.pNext = nullptr;
	createInfo.flags = 0;
	createInfo.surface = (*surface)();

	auto caps = surface->GetCapabilities();

	// Five seems like a good amount
	createInfo.minImageCount = std::max(std::min((uint32_t)5, caps.maxImageCount), caps.minImageCount);

	auto format = GetSupportedSurfaceFormat();
	createInfo.imageFormat = format.format;
	createInfo.imageColorSpace = format.colorSpace;

	if (caps.currentExtent.width != std::numeric_limits<uint32_t>::max())
	{
		// Most (all?) window managers give us no choice, so we should listen to them
		createInfo.imageExtent = caps.currentExtent;
	}
	else
	{
		createInfo.imageExtent =
		{
			std::clamp(width, caps.minImageExtent.width, caps.maxImageExtent.width),
			std::clamp(height, caps.minImageExtent.height, caps.maxImageExtent.height)
		};
	}

	if (createInfo.imageExtent.width != 0 && createInfo.imageExtent.height != 0)
	{
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;

		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = nullptr;

		createInfo.preTransform = caps.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

		createInfo.presentMode = GetSupportedPresentMode(createInfo.minImageCount >= 3);
		createInfo.clipped = VK_TRUE;
		createInfo.oldSwapchain = first ? VK_NULL_HANDLE : vkOldSwapchain;

		VKFNCHECKRETURN(device->deviceVKFN->vkCreateSwapchainKHR((*device)(), &createInfo, nullptr, &vkSwapchain));
	}

	if (!first)
	{
		// TODO: Retrire images before calling
		device->deviceVKFN->vkDestroySwapchainKHR((*device)(), vkOldSwapchain, nullptr);
	}

	first = false;
}

VkPresentModeKHR CAM::Renderer::VKSwapchain::GetSupportedPresentMode(bool mailbox)
{
	uint32_t count;

	VKFNCHECKRETURN(instance->instanceVKFN->vkGetPhysicalDeviceSurfacePresentModesKHR
	(
		device->GetPhysicalDevice(),
		(*surface)(),
		&count,
		nullptr
	));

	std::vector<VkPresentModeKHR> modes(count);

	VKFNCHECKRETURN(instance->instanceVKFN->vkGetPhysicalDeviceSurfacePresentModesKHR
	(
		device->GetPhysicalDevice(),
		(*surface)(),
		&count,
		modes.data()
	));

	if (mailbox)
	{
		for (auto& mode : modes)
		{
			if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				return VK_PRESENT_MODE_MAILBOX_KHR;
			}
		}
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

VkSurfaceFormatKHR CAM::Renderer::VKSwapchain::GetSupportedSurfaceFormat()
{
	uint32_t count;

	VKFNCHECKRETURN(instance->instanceVKFN->vkGetPhysicalDeviceSurfaceFormatsKHR
	(
		device->GetPhysicalDevice(),
		(*surface)(),
		&count,
		nullptr
	));

	std::vector<VkSurfaceFormatKHR> formats(count);

	VKFNCHECKRETURN(instance->instanceVKFN->vkGetPhysicalDeviceSurfaceFormatsKHR
	(
		device->GetPhysicalDevice(),
		(*surface)(),
		&count,
		formats.data()
	));

	if (formats.size() == 1 && formats[0].format == VK_FORMAT_UNDEFINED)
	{
		return {VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
	}

	for (auto& format : formats)
	{
		if (format.format == VK_FORMAT_R8G8B8A8_UNORM && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			return format;
		}
	}

	for (auto& format : formats)
	{
		if (format.format == VK_FORMAT_R8G8B8A8_UNORM)
		{
			return format;
		}
	}

	return formats[0];
}

CAM::Renderer::VKSwapchain::~VKSwapchain()
{
	// TODO: Retrire images before calling
	device->deviceVKFN->vkDestroySwapchainKHR((*device)(), vkSwapchain, nullptr);
}
