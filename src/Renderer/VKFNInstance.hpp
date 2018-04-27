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

#ifndef CAM_RENDERER_VKFNINSTANCE_HPP
#define CAM_RENDERER_VKFNINSTANCE_HPP

#include "Vulkan.h"

#include "../Config.hpp"

namespace CAM
{
namespace Renderer
{
class InstanceVKFN
{
	public:
	inline InstanceVKFN(VkInstance* ins) : ins(ins) {}

	#define VKFNINSTANCEPROC(x) PFN_##x x;
	#define VKFNINSTANCEPROC_VAL(x) PFN_##x x;
	#include "VKFNList.hpp"
	#undef VKFNINSTANCEPROC
	#undef VKFNINSTANCEPROC_VAL

	[[nodiscard]] inline bool InitInstanceFuncs()
	{
		#define VKFNINSTANCEPROC(x) \
		if (!(x = (PFN_##x)vkGetInstanceProcAddr(*ins, #x))) \
		{ \
			printf("Couldn't load instance func \"" #x "\"\n"); \
			return false; \
		}
		#include "VKFNList.hpp"
		#undef VKFNINSTANCEPROC

		if constexpr (CAM::Config::ValidationEnabled)
		{
			#define VKFNINSTANCEPROC_VAL(x) \
			if (!(x = (PFN_##x)vkGetInstanceProcAddr(*ins, #x))) \
			{ \
				printf("Couldn't load instance func \"" #x "\"\n"); \
				return false; \
			}
			#include "VKFNList.hpp"
			#undef VKFNINSTANCEPROC_VAL
		}

		return true;
	}

	private:
	VkInstance* ins;
};
}
}

#endif
