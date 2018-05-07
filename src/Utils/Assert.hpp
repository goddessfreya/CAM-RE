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

#ifndef CAM_UTILS_ASSERT_HPP
#define CAM_UTILS_ASSERT_HPP

#include <string>

#define ASSERT(x, y) CAM::Utils::Assert(x, "Condition `" #x "` failed at \"" __FILE__ ":" + std::to_string(__LINE__) + "\": " + y)

namespace CAM
{
namespace Utils
{
	inline void Assert(bool res, std::string message)
	{
		if (!res)
		{
			throw std::logic_error(message);
		}
	}
}
}

#endif
