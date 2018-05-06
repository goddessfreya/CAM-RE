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

#ifndef CAM_RENDERER_RENDERER_HPP
#define CAM_RENDERER_RENDERER_HPP

#include "../Jobs/Job.hpp"
#include "../Jobs/WorkerPool.hpp"

#include <cstdint>
#include <cstdio>

#include "Vulkan.h"
#include <SDL2/SDL.h>

#include "VKFNGlobal.hpp"

#include "SDLWindow.hpp"

#include "VKInstance.hpp"
#include "VKDevice.hpp"
#include "VKSurface.hpp"
#include "VKSwapchain.hpp"

#include "../Config.hpp"

namespace CAM
{
namespace Renderer
{
class Renderer
{
	public:

	Renderer(Jobs::WorkerPool* wp, Jobs::Job* thisJob);

	void DoFrame
	(
		Jobs::WorkerPool* wp,
		size_t thread,
		Jobs::Job* thisJob
	);
	bool ShouldContinue();
	void AcquireImage(Jobs::Job* thisJob);

	VKInstance* GetVKInstance() { return vkInstance.get(); }
	SDLWindow* GetSDLWindow() { return window.get(); }
	VKDevice* GetVKDevice() { return vkDevice.get(); }
	VKSurface* GetVKSurface() { return vkSurface.get(); }
	VKSwapchain* GetVKSwapchain() { return vkSwapchain.get(); }

	VKSwapchain::ImgData imgData;

	private:
	std::unique_ptr<SDLWindow> window;
	std::unique_ptr<VKInstance> vkInstance;
	std::unique_ptr<VKSurface> vkSurface;
	std::unique_ptr<VKDevice> vkDevice;
	std::unique_ptr<VKSwapchain> vkSwapchain;
	CAM::Jobs::WorkerPool* UNUSED(wp);
};
}
}

#endif
