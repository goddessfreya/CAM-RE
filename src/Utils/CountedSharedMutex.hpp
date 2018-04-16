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
 * This is a shared mutex which maintains a count of how many lockers are left
 * and whether or not its currently locked and how many shared_locks are
 * currently issued.
 */

#ifndef CAM_UTILS_COUNTEDSHAREDMUTEX_HPP
#define CAM_UTILS_COUNTEDSHAREDMUTEX_HPP

#include <shared_mutex>

namespace CAM
{
class CountedSharedMutex : public std::shared_mutex
{
	public:
	inline void lock()
	{
		++lockersLeft;
		shared_mutex::lock();
		--lockersLeft;
		uniqueLocked = true;
	}

	bool try_lock()
	{
		++lockersLeft;
		auto ret = shared_mutex::try_lock();
		if (ret)
		{
			uniqueLocked = true;
		}
		--lockersLeft;
		return ret;
	}

	inline void unlock()
	{
		shared_mutex::unlock();
		uniqueLocked = false;
	}

	inline void lock_shared()
	{
		++lockersLeft;
		shared_mutex::lock_shared();
		--lockersLeft;
		++sharedCount;
	}

	bool try_lock_shared()
	{
		++lockersLeft;
		auto ret = shared_mutex::try_lock_shared();
		if (ret)
		{
			++sharedCount;
		}
		--lockersLeft;
		return ret;
	}

	inline void unlock_shared()
	{
		shared_mutex::unlock_shared();
		--sharedCount;
	}

	inline uint32_t LockersLeft() { return lockersLeft; }

	inline uint32_t SharedCount() { return sharedCount; }
	inline bool UniqueLocked() { return uniqueLocked; }

	private:
	std::atomic<uint32_t> lockersLeft = 0;

	std::atomic<uint32_t> sharedCount = 0;
	bool uniqueLocked = false;

};

}
#endif
