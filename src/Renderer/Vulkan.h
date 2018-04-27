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

#ifndef CAM_RENDERER_VULKAN_H
#define CAM_RENDERER_VULKAN_H

#define VK_NO_PROTOTYPES 1

// TODO: Add mac stuff
// TODO: Have someone on windows test this

#if defined(_WIN32)
	#define VK_USE_PLATFORM_WIN32_KHR
#elif defined(__linux)
	#define VK_USE_PLATFORM_XCB_KHR
	#define VK_USE_PLATFORM_XLIB_KHR
#endif

#include <vulkan/vulkan.h>
#include "SDL2/SDL_loadso.h"

typedef PFN_vkVoidFunction(VKAPI_PTR *PFN_vkGetInstanceProcAddr)(VkInstance instance, const char* pName);
inline PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr;

inline void LoadVulkan()
{
	// TODO: Get correct paths for win and macs
    auto vLib = SDL_LoadObject("libvulkan.so.1");
	if (!vLib)
	{
		throw std::runtime_error("Vulkan lib not found.");
	}

	if (!(vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)SDL_LoadFunction(vLib, "vkGetInstanceProcAddr")))
	{
		throw std::runtime_error("vkGetInstanceProcAddr not found.");
	}

	SDL_UnloadObject(vLib);
}

#endif
