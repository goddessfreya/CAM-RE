#ifndef CAM_COUNTEDMUTEX_HPP
#define CAM_COUNTEDMUTEX_HPP

#include <mutex>

namespace CAM
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
#endif
