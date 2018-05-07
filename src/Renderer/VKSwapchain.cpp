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
#include "VKQueue.hpp"
#include "../Config.hpp"
#include "../Utils/ConditionalContinue.hpp"

CAM::Renderer::VKSwapchain::VKSwapchain(Jobs::WorkerPool* wp, Jobs::Job* thisJob, Renderer* parent)
	: wp(wp),
	parent(parent),
	device(parent->GetVKDevice()),
	surface(parent->GetVKSurface()),
	instance(parent->GetVKInstance()),
	window(parent->GetSDLWindow())
{
	vkSwapchain = VK_NULL_HANDLE;
	RecreateSwapchain([] (Jobs::WorkerPool*, size_t, Jobs::Job*) {}, thisJob);
	nextSync.store(0, std::memory_order_release);
}

void CAM::Renderer::VKSwapchain::RecreateSwapchain(Jobs::JobD::JobFunc postOp, Jobs::Job* thisJob)
{
	/*
	 * [[M]Get Window Size lambda] -> [postOp] -> *
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
				[this, width, height](Jobs::WorkerPool*, size_t, Jobs::Job* thisJob)
				{
					RecreateSwapchain_Internal((uint32_t)width, (uint32_t)height, thisJob);
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

	auto postOpJob = wp->GetJob
	(
		postOp,
		0,
		false
	);

	postOpJob->DependsOn(getWinSizeJob.get());
	if (!wp->SubmitJob(std::move(getWinSizeJob))) { throw std::runtime_error("Could not submit job\n"); }

	postOpJob->SameThingsDependOnMeAs(thisJob);
	if (!wp->SubmitJob(std::move(postOpJob))) { throw std::runtime_error("Could not submit job\n"); }
}

void CAM::Renderer::VKSwapchain::RecreateSwapchain_Internal(uint32_t width, uint32_t height, Jobs::Job* thisJob)
{
	std::unique_lock<std::mutex> lock(vkSwapchainMutex);

	vkOldSwapchain = VK_NULL_HANDLE;
	std::swap(vkSwapchain, vkOldSwapchain);

	VkSwapchainCreateInfoKHR createInfo;
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.pNext = nullptr;
	createInfo.flags = 0;
	createInfo.surface = (*surface)();

	auto caps = surface->GetCapabilities();

	// Seven seems like a good amount (4 + 3)
	createInfo.minImageCount = std::clamp
	(
		(uint32_t)4,
		caps.minImageCount,
		caps.maxImageCount != 0 ? caps.maxImageCount : std::numeric_limits<uint32_t>::max()
	);
	createInfo.minImageCount = std::clamp
	(
		createInfo.minImageCount + 3,
		createInfo.minImageCount,
		caps.maxImageCount != 0 ? caps.maxImageCount : std::numeric_limits<uint32_t>::max()
	);

	format = GetSupportedSurfaceFormat();
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

		// We should ignore the alpha channel, but sometimes this isn't supported
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

		// So we select the first flag we find
		const std::vector<VkCompositeAlphaFlagBitsKHR> cAlphaFlags =
		{
			VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
			VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
			VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
			VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR
		};

		for (auto& cAlphaFlag : cAlphaFlags)
		{
			if (caps.supportedCompositeAlpha & cAlphaFlag)
			{
				createInfo.compositeAlpha = cAlphaFlag;
				break;
			};
		}

		// mailbox is no good if we have less than 3 images
		createInfo.presentMode = GetSupportedPresentMode((createInfo.minImageCount - caps.minImageCount) >= 3);
		createInfo.clipped = VK_TRUE;
		createInfo.oldSwapchain = vkOldSwapchain;

		device->deviceVKFN->vkDeviceWaitIdle((*device)());
		VKFNCHECKRETURN(device->deviceVKFN->vkCreateSwapchainKHR((*device)(), &createInfo, nullptr, &vkSwapchain));
	}

	if (vkOldSwapchain != VK_NULL_HANDLE)
	{
		device->deviceVKFN->vkDeviceWaitIdle((*device)());
		swapImageDatas.clear();
		device->deviceVKFN->vkDestroySwapchainKHR((*device)(), vkOldSwapchain, nullptr);
	}

	if (createInfo.imageExtent.width != 0 && createInfo.imageExtent.height != 0)
	{
		uint32_t imgCount;
		VKFNCHECKRETURN(device->deviceVKFN->vkGetSwapchainImagesKHR
		(
			(*device)(),
			vkSwapchain,
			&imgCount,
			nullptr
		));

		std::vector<VkImage> images(imgCount);
		swapImageDatas.reserve(imgCount);

		VKFNCHECKRETURN(device->deviceVKFN->vkGetSwapchainImagesKHR
		(
			(*device)(),
			vkSwapchain,
			&imgCount,
			images.data()
		));

		VkImageViewCreateInfo imageCreateInfo;
		imageCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageCreateInfo.format = format.format;
		imageCreateInfo.components =
		{
			VK_COMPONENT_SWIZZLE_R,
			VK_COMPONENT_SWIZZLE_G,
			VK_COMPONENT_SWIZZLE_B,
			VK_COMPONENT_SWIZZLE_A
		};
		imageCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageCreateInfo.subresourceRange.baseMipLevel = 0;
		imageCreateInfo.subresourceRange.levelCount = 1;
		imageCreateInfo.subresourceRange.baseArrayLayer = 0;
		imageCreateInfo.subresourceRange.layerCount = 1;

		uint32_t index = 0;
		for(auto& image : images)
		{
			auto vImg = std::make_unique<VKImage>(wp, thisJob, parent, std::move(image));
			auto vImgP = vImg.get();
			swapImageDatas.push_back
			({
				index,
				std::move(vImg),
				std::make_unique<VKImageView>(wp, thisJob, parent, imageCreateInfo, vImgP)
			});
			swapImageSyncDatas.push_back
			({
				std::make_unique<VKSemaphore>(wp, thisJob, parent),
				std::make_unique<VKSemaphore>(wp, thisJob, parent)
			});
			++index;
		}
	}
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
	if (vkSwapchain != VK_NULL_HANDLE)
	{
		device->deviceVKFN->vkDeviceWaitIdle((*device)());
		swapImageDatas.clear();
		device->deviceVKFN->vkDestroySwapchainKHR((*device)(), vkSwapchain, nullptr);
	}
}

CAM::Renderer::VKSwapchain::ImgData CAM::Renderer::VKSwapchain::AcquireImage(Jobs::Job* thisJob, Jobs::JobD::JobFunc failOp)
{
	size_t thisSync = nextSync.load(std::memory_order_acquire);

	std::unique_lock<std::mutex> lock(vkSwapchainMutex);
	if (swapImageSyncDatas.size() != 0)
	{
		while(!std::atomic_compare_exchange_weak_explicit
		(
			&nextSync,
			&thisSync,
			(thisSync + 1) == swapImageSyncDatas.size() ? 0 : thisSync + 1,
			std::memory_order_acq_rel,
			std::memory_order_relaxed
		)) {}

		uint32_t index;
		auto sem = (*swapImageSyncDatas[thisSync].imageAvailableSemaphore)();
		sem.first.lock();

		while (true)
		{
			if (vkSwapchain != VK_NULL_HANDLE)
			{
				auto res = device->deviceVKFN->vkAcquireNextImageKHR
				(
					(*device)(),
					vkSwapchain,
					std::numeric_limits<uint32_t>::max(),
					sem.second,
					VK_NULL_HANDLE,
					&index
				);

				if (res == VK_TIMEOUT)
				{
					// Retry
					continue;
				}

				if (res == VK_SUCCESS)
				{
					return {&swapImageSyncDatas[thisSync], &swapImageDatas[index]};
				}
			}
			break;
		}
	}

	// VK_ERROR_OUT_OF_DATE_KHR or VK_SUBOPTIMAL_KHR or it doesn't exist
	lock.unlock();

	RecreateSwapchain(failOp, thisJob);
	return {nullptr, nullptr};
}

void CAM::Renderer::VKSwapchain::PresentImage(Jobs::Job* thisJob, SwapImageSyncData* sisData, SwapImageData* siData)
{
	VkPresentInfoKHR presentInfo;
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext = nullptr;

	presentInfo.swapchainCount = 1;
	presentInfo.pImageIndices = &siData->index;
	presentInfo.pResults = nullptr;

	auto sem = (*sisData->imageAvailableSemaphore)();
	auto queue = (*device->GetQueue(QueueType::Present))();
	std::unique_lock<std::mutex> lock(vkSwapchainMutex, std::defer_lock);

	std::lock(lock, sem.first, queue.second);

	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &sem.second;
	presentInfo.pSwapchains = &vkSwapchain;

	if
	(
		vkSwapchain != VK_NULL_HANDLE
		&& device->deviceVKFN->vkQueuePresentKHR
		(
			queue.first,
			&presentInfo
		) == VK_SUCCESS
	)
	{
		return;
	}

	// Won't get presented because
	// VK_ERROR_OUT_OF_DATE_KHR or VK_SUBOPTIMAL_KHR or it doesn't exist
	sem.first.unlock();
	lock.unlock();

	RecreateSwapchain([] (Jobs::WorkerPool*, size_t, Jobs::Job*) {}, thisJob);
	return;
}
