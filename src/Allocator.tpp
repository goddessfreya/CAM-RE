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
	const size_t allocationSize = 200;

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

template<typename Ret>
thread_local std::vector<std::unique_ptr<Ret>> CAM::Allocator<Ret>::retQueue;

#endif
