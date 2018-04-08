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
	static inline Ret Rand(const Ret& min, const Ret& max)
	{
		static thread_local std::mt19937 gen{std::random_device()()};
		std::uniform_int_distribution<Ret> dist(min, max);
		return dist(gen);
	}

	inline Ret operator()(const Ret& min, const Ret& max)
	{
		return ThreadSafeRandomNumberGenerator::Rand(min, max);
	}
};
}

#endif
