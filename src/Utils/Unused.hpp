/*
 * Copyright (C) 2018 Hal Gentz
 *
 * This file is part of CAM-RE.
 *
 * CAM-RE is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * Bash is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * CAM-RE. If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * Defines a couple of macros for making a variable unused.
 *
 * Use MAYBE_UNUSED when you are referring to a variable in a function.
 * Use UNUSED when referring to a variable outside a function.
 */

#ifndef CAM_UTILS_UNUSED_HPP
#define CAM_UTILS_UNUSED_HPP

#if __has_cpp_attribute(maybe_unused)
	#define MAYBE_UNUSED [[maybe_unused]]
#elif __has_cpp_attribute(gnu::unused)
	#define MAYBE_UNUSED [[gnu::unused]]
#else
	#define MAYBE_UNUSED
#endif

#define UNUSED(x) x; [[noreturn]] inline void x ## NOTUNSEDFUNC() const \
{ \
	MAYBE_UNUSED const auto& n = x; \
	throw std::logic_error("Calling this must be an accident..."); \
}

#endif
