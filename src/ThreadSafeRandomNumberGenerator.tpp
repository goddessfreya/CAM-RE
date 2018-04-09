#ifndef CAM_THREADSAFERANDOMNUMBERGENERATOR_TPP
#define CAM_THREADSAFERANDOMNUMBERGENERATOR_TPP
/*
 * This file is part of CAM-RE and is distributed under the GPLv3 License.
 * See LICENSE for more details.
 *
 * (C) 2018 Hal Gentz
 */

/*
 * This is a thead safe random number generator. It says what it does on the
 * tin, so what more could you ask for?
 */

#include <random>

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
