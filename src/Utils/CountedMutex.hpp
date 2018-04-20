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
 * This is a mutex which maintains a count of how many lockers are active.
 */

#ifndef CAM_UTILS_COUNTEDMUTEX_HPP
#define CAM_UTILS_COUNTEDMUTEX_HPP

#include <mutex>
#include <atomic>

namespace CAM
{
namespace Utils
{
class CountedMutex : public std::mutex
{
	public:
	inline void lock()
	{
		lockersLeft.fetch_add(1, std::memory_order_acquire);
		mutex::lock();
	}

	bool try_lock()
	{
		lockersLeft.fetch_add(1, std::memory_order_acquire);
		return mutex::try_lock();
	}

	inline void unlock()
	{
		mutex::unlock();
		lockersLeft.fetch_sub(1, std::memory_order_release);
	}

	[[nodiscard]] inline uint32_t LockersLeft() const { return lockersLeft.load(std::memory_order_release); }

	private:
	// uniqueLocked might be fucked up depending how the compiler reorders it
	std::atomic<uint32_t> lockersLeft = 0;
};
}
}
#endif
