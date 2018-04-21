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
 * This allows a thread to continue if the following condition has been met:
 *	- A predicator was passed
 *		- and signal was called once since the last wait from this thread
 *			- and the predicator is met
 *		- and signal is called while waiting
 *			- and the predicator is met
 *	- or a predicator wasn't passed
 *		- and signal was called once since the last wait from this thread
 *		- and signal is called while waiting
 */

#ifndef CAM_UTILS_CONDITIONALCONTINUE_HPP
#define CAM_UTILS_CONDITIONALCONTINUE_HPP

#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <map>

namespace CAM
{
namespace Utils
{
class ConditionalContinue
{
	private:
	int& WaitLogic()
	{
		auto thisThread = std::this_thread::get_id();

		if (waitMap.find(thisThread) == std::end(waitMap))
		{
			waitMap.insert({thisThread, 0});
		}

		auto& ret = waitMap.at(thisThread);
		++ret;

		return ret;
	}
	public:
	void Wait()
	{
		auto v = WaitLogic();

		std::unique_lock<std::mutex> lock(cvM);
		if (v <= signals)
		{
			v = signals;
			return;
		}

		cv.wait(lock);
	}

	template<class Pred>
	void Wait(Pred pred)
	{
		auto v = WaitLogic();

		std::unique_lock<std::mutex> lock(cvM);
		if (v <= signals && pred())
		{
			v = signals;
			return;
		}

		cv.wait(lock, [&pred, this] { return pred() || shutingDown; });
	}

	void Signal()
	{
		std::unique_lock<std::mutex> lock(cvM);
		++signals;
		if (signals < 0)
		{
			for(auto& v : waitMap)
			{
				v.second = 0;
			}

			signals = 1;
		}
		cv.notify_all();
	}

	void Reset()
	{
		shutingDown = true;
		std::unique_lock<std::mutex> lock(cvM);
		cv.notify_all();

		signals = 0;
		waitMap.clear();

		shutingDown = false;
	}

	private:
	std::atomic<int> signals = 0;
	std::atomic<bool> shutingDown = false;
	std::map<std::thread::id, int> waitMap;

	std::condition_variable cv;
	std::mutex cvM;
};
}
}
#endif
