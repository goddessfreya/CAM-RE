#ifndef CAM_UTILS_UNUSED_HPP
#define CAM_UTILS_UNUSED_HPP

/*
 * This file is part of CAM-RE and is distributed under the GPLv3 License.
 * See LICENSE for more details.
 *
 * (C) 2018 Hal Gentz
 */

/*
 * Defines a couple of macros for making a variable unused.
 *
 * Use MAYBE_UNUSED when you are refering to a variable in a function.
 * Use UNUSED when refering to a variable outside a function.
 */

#if __has_cpp_attribute(maybe_unused)
	#define MAYBE_UNUSED [[maybe_unused]]
#elif __has_cpp_attribute(gnu::unused)
	#define MAYBE_UNUSED [[gnu::unused]]
#else
	#define MAYBE_UNUSED
#endif

#define UNUSED(x) x; inline void x ## Unused() const noexcept { MAYBE_UNUSED const auto& n = &x; }

#endif
