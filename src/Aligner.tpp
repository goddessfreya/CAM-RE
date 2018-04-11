#ifndef CAM_ALIGNER_TPP
#define CAM_ALIGNER_TPP

/*
 * This file is part of CAM-RE and is distributed under the GPLv3 License.
 * See LICENSE for more details.
 *
 * (C) 2018 Hal Gentz
 */

/*
 * Aligns an objects to dSize.
 *
 * dSize should be set to 'std::hardware_destructive_interference_size' if your
 * vendor supports it, mine didn't so I'm using my cache size, which is 64, and
 * I'm asuming yours is too!
 *
 * This assumption should hold up for most (all?) x86_64 PCs. Worst case
 * scenario is we waste memory.
 */

#include <cstdint>
#include <type_traits>
#include <array>

#if __has_cpp_attribute(maybe_unused)
	#define MAYBE_UNUSED [[maybe_unused]]
#elif __has_cpp_attribute(gnu::unused)
	#define MAYBE_UNUSED [[gnu::unused]]
#else
	#define MAYBE_UNUSED
#endif

#define UNUSED MAYBE_UNUSED

namespace CAM
{
template<typename T>
class PaddingNeeded
{
	public:
	static constexpr size_t memUsed = sizeof(T);
	static constexpr size_t dSize = 64;

	static constexpr size_t dSizeNeeded = memUsed / dSize + 1;
	static constexpr size_t paddingSize = (dSizeNeeded * dSize) - memUsed;

	static constexpr bool True = ((paddingSize % 64) == 0) ? false : true;
};

template<class T, class Enable = void>
class Aligner : public T {};

template<class T>
class Aligner<T, typename std::enable_if<PaddingNeeded<T>::True>::type> : public T
{
	std::array<uint8_t, PaddingNeeded<T>::paddingSize> padding;
	void UnusedToMakeGCCHappy()
	{
		UNUSED uint8_t unused = &padding[0];
	}
};

}

#endif
