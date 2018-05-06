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

#ifndef CAM_RENDERER_VKFNGLOBAL_HPP
#define CAM_RENDERER_VKFNGLOBAL_HPP

#include "Vulkan.h"

namespace CAM
{
namespace VKFN
{
	#define VKFNGLOBALPROC(x) inline PFN_##x x;
	#include "VKFNList.hpp"
	#undef VKFNGLOBALPROC

	// Main thread only
	[[nodiscard]] inline bool InitGlobalFuncs()
	{
		LoadVulkan();

		#define VKFNGLOBALPROC(x) \
		if (!(x = (PFN_##x)vkGetInstanceProcAddr(nullptr, #x))) \
		{ \
			printf("Couldn't load global func \"" #x "\"\n"); \
			return false; \
		}
		#include "VKFNList.hpp"
		#undef VKFNGLOBALPROC

		return true;
	}
}
}

#endif
