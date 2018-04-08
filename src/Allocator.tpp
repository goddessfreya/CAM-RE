#ifndef CAM_ALLOCATOR_TPP
#define CAM_ALLOCATOR_TPP

#include <thread>
#include <map>
#include <memory>
#include <shared_mutex>

namespace CAM
{
template<typename Ret>
class Allocator
{
	public:
	const size_t allocationSize = 100;

	inline std::vector<std::unique_ptr<Ret>>* GetRetQueue()
	{
		static std::shared_mutex hashMapMutex;
		static std::map<std::thread::id, std::unique_ptr<std::vector<std::unique_ptr<Ret> > > > retQueueTable;

		auto shmm = std::make_unique<std::shared_lock<std::shared_mutex>>(hashMapMutex);
		auto retQueue = retQueueTable.find(std::this_thread::get_id());

		if (retQueue == std::end(retQueueTable))
		{
			shmm = nullptr;
			auto uhmm = std::make_unique<std::unique_lock<std::shared_mutex>>(hashMapMutex);
			retQueueTable[std::this_thread::get_id()] = std::make_unique<std::vector<std::unique_ptr<Ret>>>();
			uhmm = nullptr;
			return GetRetQueue();
		}

		return retQueue->second.get();
	}

	inline void Return(std::unique_ptr<Ret> ret)
	{
		auto retQueue = GetRetQueue();
		if (retQueue->size() >= allocationSize * 10)
		{
			return;
		}
		retQueue->push_back(std::move(ret));
	}

	template<typename... Args>
	inline std::unique_ptr<Ret> Get(Args&&... args)
	{
		auto retQueue = GetRetQueue();

		if (retQueue->empty())
		{
			retQueue->reserve(allocationSize);

			for (size_t i = 0; i < allocationSize - 1; ++i)
			{
				retQueue->push_back(std::make_unique<Ret>());
			}
			return std::make_unique<Ret>(std::forward<Args>(args)...);
		}

		auto ret = std::move(retQueue->back());
		retQueue->pop_back();
		ret->Reset(std::forward<Args>(args)...);
		return ret;
	}

	template<typename... Args>
	inline std::unique_ptr<Ret> operator()(Args&&... args)
	{
		return Get(std::forward<Args>(args)...);
	}
};
}

#endif
