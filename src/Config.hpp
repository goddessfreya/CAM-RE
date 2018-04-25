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

#ifndef CAM_CONFIG_HPP
#define CAM_CONFIG_HPP

namespace CAM
{
namespace Config
{
static constexpr bool ValidationEnabled = true;
//static constexpr bool ValidationEnabled = false;

const size_t ThreadCount = std::thread::hardware_concurrency() * 2 + 1;
//const size_t ThreadCount = 1;
}
}

#endif
