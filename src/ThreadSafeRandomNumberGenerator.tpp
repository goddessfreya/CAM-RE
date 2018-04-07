#ifndef CAM_THREADSAFERANDOMNUMBERGENERATOR_TPP
#define CAM_THREADSAFERANDOMNUMBERGENERATOR_TPP

#include <thread>
#include <random>
#include <map>
#include <memory>
#include <shared_mutex>

namespace CAM
{
template<typename Ret>
class ThreadSafeRandomNumberGenerator
{
	public:
	static inline std::mt19937* GetGen()
	{
		static std::shared_mutex hashMapMutex;
		static std::map<std::thread::id, std::unique_ptr<std::mt19937>> genTable;

		auto shmm = std::make_unique<std::shared_lock<std::shared_mutex>>(hashMapMutex);
		auto gen = genTable.find(std::this_thread::get_id());

		if (gen == std::end(genTable))
		{
			shmm = nullptr;
			auto uhmm = std::make_unique<std::unique_lock<std::shared_mutex>>(hashMapMutex);
			std::random_device rd;
			genTable[std::this_thread::get_id()] = std::make_unique<std::mt19937>(rd());
			return (*genTable.find(std::this_thread::get_id())).second.get();
		}

		return (*gen).second.get();
	}

	static inline Ret Rand(const Ret& min, const Ret& max)
	{
		auto gen = GetGen();
		std::uniform_int_distribution<Ret> dist(min, max);
		return dist(*gen);
	}

	inline Ret operator()(const Ret& min, const Ret& max)
	{
		return ThreadSafeRandomNumberGenerator::Rand(min, max);
	}
};
}

#endif
