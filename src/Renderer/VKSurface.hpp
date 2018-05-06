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

#ifndef CAM_RENDERER_VKSURFACE_HPP
#define CAM_RENDERER_VKSURFACE_HPP

#include "../Jobs/Job.hpp"
#include "../Jobs/WorkerPool.hpp"

#include <cstdint>
#include <cstring>
#include <cstdio>

#include "Vulkan.h"
#include "SDL2/SDL.h"

namespace CAM
{
namespace Renderer
{
class SDLWindow;
class VKInstance;
class Renderer;

class VKSurface
{
	public:
	VKSurface(Jobs::WorkerPool* wp, Jobs::Job* thisJob, Renderer* parent);
	~VKSurface();

	VKSurface(const VKSurface&) = delete;
	VKSurface(VKSurface&&) = default;
	VKSurface& operator=(const VKSurface&)& = delete;
	VKSurface& operator=(VKSurface&&)& = default;

	inline VkSurfaceKHR& operator()() { return vkSurface; }
	inline const VkSurfaceCapabilitiesKHR& GetCapabilities() const { return surfaceCapabilities; }
	void UpdateCaps();

	private:
	CAM::Jobs::WorkerPool* UNUSED(wp);

	VkSurfaceKHR vkSurface;

	Renderer* UNUSED(parent);
	VKInstance* instance;
	SDLWindow* window;

	VkSurfaceCapabilitiesKHR surfaceCapabilities;
};
}
}

#endif
