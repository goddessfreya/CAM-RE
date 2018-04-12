#ifndef CAM_UTILS_UNUSED_HPP
#define CAM_UTILS_UNUSED_HPP

/*
 * This file is part of CAM-RE and is distributed under the GPLv3 License.
 * See LICENSE for more details.
 *
 * (C) 2018 Hal Gentz
 */

#if __has_cpp_attribute(maybe_unused)
	#define MAYBE_UNUSED [[maybe_unused]]
#elif __has_cpp_attribute(gnu::unused)
	#define MAYBE_UNUSED [[gnu::unused]]
#else
	#define MAYBE_UNUSED
#endif

#define UNUSED MAYBE_UNUSED

#endif
