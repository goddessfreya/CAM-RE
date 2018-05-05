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

#include "Vulkan.h"
#include "SDL2/SDL.h"

namespace CAM
{
namespace Renderer
{
class SDLWindow;
class VKInstance;
class Renderer;

class VKSwapchain
{
	public:
	VKSwapchain(Jobs::WorkerPool* wp, Jobs::Job* thisJob, Renderer* parent);
	~VKSwapchain();

	VKSwapchain(const VKSwapchain&) = default;
	VKSwapchain(VKSwapchain&&) = delete;
	VKSwapchain& operator=(const VKSwapchain&)& = default;
	VKSwapchain& operator=(VKSwapchain&&)& = delete;

	inline VkSwapchainKHR& operator()() { return vkSwapchain; }
	void RecreateSwapchain(Jobs::Job* thisJob);

	private:
	void RecreateSwapchain_Internal(uint32_t width, uint32_t height);

	VkPresentModeKHR GetSupportedPresentMode(bool mailbox);
	VkSurfaceFormatKHR GetSupportedSurfaceFormat();

	CAM::Jobs::WorkerPool* wp;
	bool first = true;

	VkSwapchainKHR vkSwapchain;
	VkSwapchainKHR vkOldSwapchain;

	Renderer* UNUSED(parent);
	VKDevice* device;
	VKSurface* surface;
	VKInstance* instance;
	SDLWindow* window;
};
}
}

#endif
