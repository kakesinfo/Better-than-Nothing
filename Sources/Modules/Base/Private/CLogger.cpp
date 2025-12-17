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

#include "Module_Base/CLogger.hpp"

#include <iostream>
#include <chrono>
#include <filesystem>
#include <iomanip>

namespace BetterThanNothing
{
	std::uint8_t CLogger::s_currentLogLevel = Info;
	std::mutex CLogger::s_printMutex {};

	std::string CLogger::s_getTimestamp()
	{
		const auto now = std::chrono::system_clock::now();
		const auto time = std::chrono::system_clock::to_time_t(now);

		std::stringstream ss;
		ss << std::put_time(std::localtime(&time), "[%Y-%m-%d %X]");
		return ss.str();
	}

	std::string CLogger::s_formatSourceFile(const std::source_location& location)
	{
		const std::filesystem::path fullPath(location.file_name());

		const std::string fileName = fullPath.filename().string();
		const std::string line = std::to_string(location.line());

		return "[" + fileName + ":" + line + "]";
	}

	std::string CLogger::s_sanitizeMessage(std::string_view message)
	{
		std::string s(message);
		std::erase_if(s, [](const unsigned char c) {
			return std::iscntrl(c);
		});
		return s;
	}

	void CLogger::s_print(const std::uint8_t level, const std::source_location& location, const std::string_view message)
	{
		if (level < s_currentLogLevel)
			return;

		std::lock_guard lock(s_printMutex);

		std::string levelStr;
		std::string levelColor;

		switch (level)
		{
			case Debug:		levelStr = "[DEBUG]",	levelColor = "\033[90m"; break;
			case Info:		levelStr = "[INFO]",	levelColor = "\033[36m"; break;
			case Success:	levelStr = "[SUCCESS]",	levelColor = "\033[32m"; break;
			case Warning:	levelStr = "[WARNING]",	levelColor = "\033[33m"; break;
			case Error:		levelStr = "[ERROR]",	levelColor = "\033[31m"; break;
			default: break;
		}

		std::cout << "\033[37m"	<< std::left << s_getTimestamp() << " ";
		std::cout << levelColor	<< std::left << std::setw(10) << levelStr;
		std::cout << "\033[37m"	<< std::left << std::setw(35) << s_formatSourceFile(location);
		std::cout << "\033[0m"	<< std::left << s_sanitizeMessage(message);
		std::cout << std::endl;
	}
}