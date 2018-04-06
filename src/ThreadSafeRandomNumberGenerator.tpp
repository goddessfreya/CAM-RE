#ifndef CAM_THREADSAFERANDOMNUMBERGENERATOR_TPP
#define CAM_THREADSAFERANDOMNUMBERGENERATOR_TPP

#include <thread>
#include <random>
#include <map>
#include <memory>

namespace CAM
{
template<typename Ret>
class ThreadSafeRandomNumberGenerator
{
	public:
	static inline std::mt19937* GetGen()
	{
		static std::map<std::thread::id, std::unique_ptr<std::mt19937>> genTable;
		auto gen = genTable.find(std::this_thread::get_id());
		if (gen == std::end(genTable))
		{
			std::random_device rd;
			genTable[std::this_thread::get_id()] = std::make_unique<std::mt19937>(rd());
			return (*genTable.find(std::this_thread::get_id())).second.get();
		}

		return (*gen).second.get();
	}

	inline Ret Rand(const Ret& min, const Ret& max)
	{
		auto gen = GetGen();
		std::uniform_int_distribution<Ret> dist(min, max);
		return dist(*gen);
	}

	inline Ret operator()(const Ret& min, const Ret& max)
	{
		return Rand(min, max);
	}
};
}

#endif
