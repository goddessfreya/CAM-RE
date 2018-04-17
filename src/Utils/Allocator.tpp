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
 * This allocator first tries to reuse things returned to it from the calling
 * thread before allocating new things. It will allocate allocationSize objects
 * at a time and will store up to 10 times allocationSize.
 *
 * Separate threads use a separate storage under the hood, this removes the need
 * for mutexes and the shebang.
 *
 * Ret must implement at least one empty constructor. For every possible call
 * of Get(Args) Ret must implement both a constructor which receives Args and a
 * Reset function which receives Args.
 */

#ifndef CAM_UTILS_ALLOCATOR_TPP
#define CAM_UTILS_ALLOCATOR_TPP

#include <memory>
#include <vector>
#include <cstdint>

namespace CAM
{
namespace Utils
{
template<typename Ret>
class Allocator
{
	public:
	const size_t allocationSize = 1;

	inline void Return(std::unique_ptr<Ret> ret)
	{
		if (retQueue.size() >= allocationSize * 10)
		{
			return;
		}
		retQueue.push_back(std::move(ret));
	}

	template<typename... Args>
	inline std::unique_ptr<Ret> Get(Args&&... args)
	{
		if (retQueue.empty())
		{
			retQueue.reserve(allocationSize);

			for (size_t i = 0; i < allocationSize - 1; ++i)
			{
				retQueue.push_back(std::make_unique<Ret>());
			}
			return std::make_unique<Ret>(std::forward<Args>(args)...);
		}

		auto ret = std::move(retQueue.back());
		retQueue.pop_back();
		ret->Reset(std::forward<Args>(args)...);
		return ret;
	}

	template<typename... Args>
	inline std::unique_ptr<Ret> operator()(Args&&... args)
	{
		return Get(std::forward<Args>(args)...);
	}
	private:

	static thread_local std::vector<std::unique_ptr<Ret>> retQueue;
};
}
}

template<typename Ret>
thread_local std::vector<std::unique_ptr<Ret>> CAM::Utils::Allocator<Ret>::retQueue;

#endif
