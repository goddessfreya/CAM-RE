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

#ifndef CAM_RENDERER_VKFNDEVICE_HPP
#define CAM_RENDERER_VKFNDEVICE_HPP

#include "Vulkan.h"

namespace CAM
{
namespace Renderer
{
class DeviceVKFN
{
	public:
	inline DeviceVKFN(VkDevice* dev, PFN_vkGetDeviceProcAddr vkGetDeviceProcAddr)
		: dev(dev),
		vkGetDeviceProcAddr(vkGetDeviceProcAddr)
	{}

	#define VKFNDEVICEPROC(x) PFN_##x x;
	#include "VKFNList.hpp"
	#undef VKFNDEVICEPROC

	[[nodiscard]] inline bool InitDeviceFuncs()
	{
		#define VKFNDEVICEPROC(x) \
		if (!(x = (PFN_##x)vkGetDeviceProcAddr(*dev, #x))) \
		{ \
			printf("Couldn't load instance func \"" #x "\"\n"); \
			return false; \
		}
		#include "VKFNList.hpp"
		#undef VKFNDEVICEPROC

		return true;
	}

	private:
	VkDevice* dev;
	PFN_vkGetDeviceProcAddr vkGetDeviceProcAddr;
};
}
}

#endif
