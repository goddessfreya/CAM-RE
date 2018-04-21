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
 * This is a shared mutex which maintains a count of how many lockers are active.
 */

#ifndef CAM_UTILS_COUNTEDSHAREDMUTEX_HPP
#define CAM_UTILS_COUNTEDSHAREDMUTEX_HPP

#include <shared_mutex>
#include <atomic>

namespace CAM
{
namespace Utils
{
class CountedSharedMutex : public std::shared_mutex
{
	public:
	inline void lock()
	{
		lockersLeft.fetch_add(1, std::memory_order_acquire);
		shared_mutex::lock();
	}

	bool try_lock()
	{
		lockersLeft.fetch_add(1, std::memory_order_acquire);
		return shared_mutex::try_lock();
	}

	inline void unlock()
	{
		shared_mutex::unlock();
		lockersLeft.fetch_sub(1, std::memory_order_release);
	}

	inline void lock_shared()
	{
		sharedLockersLeft.fetch_add(1, std::memory_order_acquire);
		shared_mutex::lock_shared();
	}

	bool try_lock_shared()
	{
		sharedLockersLeft.fetch_add(1, std::memory_order_acquire);
		return shared_mutex::try_lock_shared();
	}

	inline void unlock_shared()
	{
		shared_mutex::unlock_shared();
		sharedLockersLeft.fetch_sub(1, std::memory_order_release);
	}

	[[nodiscard]] inline uint32_t LockersLeft() const { return lockersLeft.load(std::memory_order_acquire); }
	[[nodiscard]] inline uint32_t SharedLockersLeft() const { return sharedLockersLeft.load(std::memory_order_acquire); }

	private:
	std::atomic<uint32_t> lockersLeft = 0;
	std::atomic<uint32_t> sharedLockersLeft = 0;
};
}
}
#endif
