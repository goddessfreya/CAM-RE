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

#ifndef CAM_VERSIONNUMBER_HPP
#define CAM_VERSIONNUMBER_HPP

#include <cstdint>
#include <type_traits>

namespace CAM
{
namespace Version
{
	constexpr char time[] = __TIME__;
	constexpr char date[] = __DATE__;

	constexpr uint32_t sixthSecs = ((time[6] - '0') * 10 + (time[7] - '0') - 1) / 6;
	constexpr uint32_t mins = ((time[3] - '0') * 10 + (time[4] - '0') - 1) * 10;
	constexpr uint32_t hrs = ((time[0] - '0') * 10 + (time[1] - '0')) * 10 * 60;

	constexpr uint32_t days = ((date[4] - '0') * 10 + (date[5] - '0') - 1) * 10 * 60 * 24;

	// good for 800 years
	constexpr uint32_t years =
	(
		(date[8] - '0') * 100
		+ (date[9] - '0') * 10
		+ (date[10] - '0')
	) * 10 * 60 * 24 * 31 * 12;

	template <const char, const char, const char>
	class VerInt
	{
		static_assert("What month is this?");
	};

	template <>
	class VerInt<'J', 'a', 'n'>
	{
		public:
		static constexpr uint32_t months = 0 * 10 * 60 * 24 * 31 * 12;
	};
	template <>
	class VerInt<'F', 'e', 'b'>
	{
		public:
		static constexpr uint32_t months = 1 * 10 * 60 * 24 * 31 * 12;
	};
	template <>
	class VerInt<'M', 'a', 'r'>
	{
		public:
		static constexpr uint32_t months = 2 * 10 * 60 * 24 * 31 * 12;
	};
	template <>
	class VerInt<'A', 'p', 'r'>
	{
		public:
		static constexpr uint32_t months = 3 * 10 * 60 * 24 * 31 * 12;
	};
	template <>
	class VerInt<'M', 'a', 'y'>
	{
		public:
		static constexpr uint32_t months = 4 * 10 * 60 * 24 * 31 * 12;
	};
	template <>
	class VerInt<'J', 'u', 'n'>
	{
		public:
		static constexpr uint32_t months = 5 * 10 * 60 * 24 * 31 * 12;
	};
	template <>
	class VerInt<'J', 'u', 'l'>
	{
		public:
		static constexpr uint32_t months = 6 * 10 * 60 * 24 * 31 * 12;
	};
	template <>
	class VerInt<'A', 'u', 'g'>
	{
		public:
		static constexpr uint32_t months = 7 * 10 * 60 * 24 * 31 * 12;
	};
	template <>
	class VerInt<'S', 'e', 'p'>
	{
		public:
		static constexpr uint32_t months = 8 * 10 * 60 * 24 * 31 * 12;
	};
	template <>
	class VerInt<'O', 'c', 't'>
	{
		public:
		static constexpr uint32_t months = 9 * 10 * 60 * 24 * 31 * 12;
	};
	template <>
	class VerInt<'N', 'o', 'v'>
	{
		public:
		static constexpr uint32_t months = 10 * 10 * 60 * 24 * 31 * 12;
	};
	template <>
	class VerInt<'D', 'e', 'c'>
	{
		public:
		static constexpr uint32_t months = 11 * 10 * 60 * 24 * 31 * 12;
	};

	class Ver : public VerInt<date[0], date[1], date[2]>
	{
		public:
		static constexpr uint32_t ver = sixthSecs + mins + hrs + days + months + years;
	};
}
}

#endif
