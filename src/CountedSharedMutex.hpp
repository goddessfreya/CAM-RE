#ifndef CAM_COUNTEDSHAREDMUTEX_HPP
#define CAM_COUNTEDSHAREDMUTEX_HPP

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
