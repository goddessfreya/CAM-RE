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

#ifndef CAM_RENDERER_VKSWAPCHAIN_HPP
#define CAM_RENDERER_VKSWAPCHAIN_HPP

#include "../Jobs/Job.hpp"
#include "../Jobs/WorkerPool.hpp"

#include <cstdint>
#include <cstring>
#include <cstdio>
#include <algorithm>
#include <mutex>
#include <atomic>

#include "Vulkan.h"
#include "SDL2/SDL.h"

#include "VKFence.hpp"
#include "VKSemaphore.hpp"
#include "VKImage.hpp"
#include "VKImageView.hpp"

namespace CAM
{
namespace Renderer
{
class SDLWindow;
class VKInstance;
class Renderer;

struct SwapImageData
{
	SwapImageData
	(
		uint32_t index,
		std::unique_ptr<VKImage>&& image,
		std::unique_ptr<VKImageView>&& imageView
	) : index(index),
		image(std::move(image)),
		imageView(std::move(imageView))
	{}

	uint32_t index;
	std::unique_ptr<VKImage> image;
	std::unique_ptr<VKImageView> imageView;
};

struct SwapImageSyncData
{
	SwapImageSyncData
	(
		std::unique_ptr<VKSemaphore>&& imageAvailableSemaphore,
		std::unique_ptr<VKSemaphore>&& presentCompleteSemaphore
	) : imageAvailableSemaphore(std::move(imageAvailableSemaphore)),
		presentCompleteSemaphore(std::move(presentCompleteSemaphore))
	{}

	std::unique_ptr<VKSemaphore> imageAvailableSemaphore;
	std::unique_ptr<VKSemaphore> presentCompleteSemaphore;
};

class VKSwapchain
{
	public:
	using ImgData = std::pair<SwapImageSyncData*, SwapImageData*>;

	VKSwapchain(Jobs::WorkerPool* wp, Jobs::Job* thisJob, Renderer* parent);
	~VKSwapchain();

	VKSwapchain(const VKSwapchain&) = delete;
	VKSwapchain(VKSwapchain&&) = default;
	VKSwapchain& operator=(const VKSwapchain&)& = delete;
	VKSwapchain& operator=(VKSwapchain&&)& = default;

	inline VkSwapchainKHR& operator()() { return vkSwapchain; }
	void RecreateSwapchain(Jobs::JobD::JobFunc postOp, Jobs::Job* thisJob);

	// Can invalidate prev SwapImage{,Sync}Data-s returned by AcquireImage
	ImgData AcquireImage(Jobs::Job* thisJob, Jobs::JobD::JobFunc failOp);

	// Can invalidate prev SwapImage{,Sync}Data-s returned by AcquireImage
	// Image must be owned by the present queue
	void PresentImage(Jobs::Job* thisJob, SwapImageSyncData* sisData, SwapImageData* siData);

	private:
	void RecreateSwapchain_Internal(uint32_t width, uint32_t height, Jobs::Job* thisJob);

	VkPresentModeKHR GetSupportedPresentMode(bool mailbox);
	VkSurfaceFormatKHR GetSupportedSurfaceFormat();

	CAM::Jobs::WorkerPool* wp;

	VkSwapchainKHR vkSwapchain;
	VkSwapchainKHR vkOldSwapchain;
	mutable std::mutex vkSwapchainMutex;

	Renderer* parent;
	VKDevice* device;
	VKSurface* surface;
	VKInstance* instance;
	SDLWindow* window;

	std::vector<SwapImageData> swapImageDatas;
	std::vector<SwapImageSyncData> swapImageSyncDatas;
	VkSurfaceFormatKHR format;

	std::atomic<size_t> nextSync;
};
}
}

#endif
