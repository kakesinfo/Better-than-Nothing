/*
 *	Better than Nothing
 *	Copyright (C) 2025 kakesinfo
 *
 *	This program is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 * 	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 * 	along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <cstdint>
#include <format>
#include <mutex>
#include <source_location>
#include <string_view>

namespace BetterThanNothing
{
	static constexpr std::uint8_t DEBUG = 0;
	static constexpr std::uint8_t INFO = 1;
	static constexpr std::uint8_t SUCCESS = 2;
	static constexpr std::uint8_t WARNING = 3;
	static constexpr std::uint8_t ERROR = 4;

	struct LogFormat
	{
		std::string_view	 format;
		std::source_location location;

		template <typename T>
		consteval LogFormat(const T& s, const std::source_location l = std::source_location::current()) : format(s), location(l)
		{}
	};

	class Logger
	{
	public:
		template <std::uint8_t Level, typename... Args>
		static void Log(const LogFormat arg, Args&&... args)
		{
			Print(Level, arg.location, std::vformat(arg.format, std::make_format_args(args...)));
		}

		static void SetLogLevel(const std::uint8_t level) { s_currentLogLevel = level; }

	private:
		static std::string GetTimestamp();
		static std::string FormatSourceFile(const std::source_location& location);
		static std::string SanitizeMessage(std::string_view message);
		static void		   Print(std::uint8_t level, const std::source_location& location, std::string_view message);

		static std::uint8_t s_currentLogLevel;
		static std::mutex	s_printMutex;
	};
}
