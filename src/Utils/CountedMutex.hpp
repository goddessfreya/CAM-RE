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
 * This is a mutex which maintains a count of how many lockers are left and
 * whether or not its currently locked.
 */

#ifndef CAM_UTILS_COUNTEDMUTEX_HPP
#define CAM_UTILS_COUNTEDMUTEX_HPP

#include <mutex>

namespace CAM
{
namespace Utils
{
class CountedMutex : public std::mutex
{
	public:
	inline void lock()
	{
		++lockersLeft;
		mutex::lock();
		--lockersLeft;
		uniqueLocked = true;
	}

	bool try_lock()
	{
		++lockersLeft;
		auto ret = mutex::try_lock();
		if (ret)
		{
			uniqueLocked = true;
		}
		--lockersLeft;
		return ret;
	}

	inline void unlock()
	{
		mutex::unlock();
		uniqueLocked = false;
	}

	inline uint32_t LockersLeft() { return lockersLeft; }
	inline bool UniqueLocked() { return uniqueLocked; }

	private:
	std::atomic<uint32_t> lockersLeft = 0;
	bool uniqueLocked = false;

};
}
}
#endif
