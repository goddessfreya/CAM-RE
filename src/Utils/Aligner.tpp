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

/*
 * Aligns an objects to dSize.
 *
 * dSize should be set to 'std::hardware_destructive_interference_size' if your
 * vendor supports it, mine didn't so I'm using my cache size, which is 64, and
 * I'm assuming yours is too!
 *
 * This assumption should hold up for most (all?) x86_64 PCs. Worst case
 * scenario is we waste memory.
 */

#ifndef CAM_UTILS_ALIGNER_TPP
#define CAM_UTILS_ALIGNER_TPP

#include <cstdint>
#include <type_traits>
#include <array>

#include "Unused.hpp"

namespace CAM
{
template<typename T>
class PaddingNeeded
{
	public:
	static constexpr size_t memUsed = sizeof(T);
	static constexpr size_t dSize = 64;

	static constexpr size_t dSizeNeeded = memUsed / dSize + 1;
	static constexpr size_t paddingSize = (dSizeNeeded * dSize) - memUsed;

	static constexpr bool True = ((paddingSize % 64) == 0) ? false : true;
};

template<class T, class Enable = void>
class Aligner : public T {};

template<class T>
class Aligner<T, typename std::enable_if<PaddingNeeded<T>::True>::type> : public T
{
	std::array<uint8_t, PaddingNeeded<T>::paddingSize> UNUSED(padding);
};

}

#endif
