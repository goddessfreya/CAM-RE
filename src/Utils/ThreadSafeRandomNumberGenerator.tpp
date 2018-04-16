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
 * A thread safe random number generator.
 */

#ifndef CAM_UTILS_THREADSAFERANDOMNUMBERGENERATOR_TPP
#define CAM_UTILS_THREADSAFERANDOMNUMBERGENERATOR_TPP

#include <random>

namespace CAM
{
template<typename Ret>
class ThreadSafeRandomNumberGenerator
{
	public:
	static inline Ret Rand(const Ret& min, const Ret& max)
	{
		static thread_local std::mt19937 gen{std::random_device()()};
		std::uniform_int_distribution<Ret> dist(min, max);
		return dist(gen);
	}

	inline Ret operator()(const Ret& min, const Ret& max)
	{
		return ThreadSafeRandomNumberGenerator::Rand(min, max);
	}
};
}

#endif
