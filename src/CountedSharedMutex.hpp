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
		shared_mutex::lock();
		uniqueLocked = true;
	}

	bool try_lock()
	{
		auto ret = shared_mutex::try_lock();
		if (ret)
		{
			uniqueLocked = true;
		}

		return ret;
	}

	inline void unlock()
	{
		shared_mutex::unlock();
		uniqueLocked = false;
	}

	inline void lock_shared()
	{
		shared_mutex::lock_shared();
		++sharedCount;
	}

	bool try_lock_shared()
	{
		auto ret = shared_mutex::try_lock_shared();
		if (ret)
		{
			++sharedCount;
		}
		return ret;
	}

	inline void unlock_shared()
	{
		shared_mutex::unlock_shared();
		--sharedCount;
	}

	inline uint32_t SharedCount() { return sharedCount; }
	inline bool UniqueLocked() { return uniqueLocked; }

	private:
	std::atomic<uint32_t> sharedCount;
	bool uniqueLocked;

};

}
#endif
