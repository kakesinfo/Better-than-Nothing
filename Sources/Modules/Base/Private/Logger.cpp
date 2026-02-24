/*
 *	Better than Nothing
 *	Copyright (C) 2026 kakesinfo
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

#include "Module_Base/Logger.hpp"

#include <chrono>
#include <filesystem>
#include <iomanip>
#include <iostream>

namespace BetterThanNothing
{
	uint8_t	   Logger::s_currentLogLevel = INFO;
	std::mutex Logger::s_printMutex{};

	std::string Logger::GetTimestamp()
	{
		const auto now = std::chrono::system_clock::now();
		const auto time = std::chrono::system_clock::to_time_t(now);

		std::stringstream ss;
		ss << std::put_time(std::localtime(&time), "[%Y-%m-%d %X]");
		return ss.str();
	}

	std::string Logger::FormatSourceFile(const std::source_location& location)
	{
		const std::filesystem::path fullPath(location.file_name());

		const std::string fileName = fullPath.filename().string();
		const std::string line = std::to_string(location.line());

		return "[" + fileName + ":" + line + "]";
	}

	std::string Logger::SanitizeMessage(std::string_view message)
	{
		std::string s(message);
		std::erase_if(s, [](const unsigned char c) { return std::iscntrl(c); });
		return s;
	}

	void Logger::Print(const uint8_t level, const std::source_location& location, const std::string_view message)
	{
		if (level < s_currentLogLevel)
			return;

		std::lock_guard lock(s_printMutex);

		std::string levelStr;
		std::string levelColor;

		switch (level)
		{
			case DEBUG:
				levelStr = "[DEBUG]", levelColor = "\033[90m";
				break;
			case INFO:
				levelStr = "[INFO]", levelColor = "\033[36m";
				break;
			case SUCCESS:
				levelStr = "[SUCCESS]", levelColor = "\033[32m";
				break;
			case WARNING:
				levelStr = "[WARNING]", levelColor = "\033[33m";
				break;
			case ERROR:
				levelStr = "[ERROR]", levelColor = "\033[31m";
				break;
			default:
				break;
		}

		std::cout << "\033[37m" << std::left << GetTimestamp() << " ";
		std::cout << levelColor << std::left << std::setw(10) << levelStr;
		std::cout << "\033[37m" << std::left << std::setw(35) << FormatSourceFile(location);
		std::cout << "\033[0m" << std::left << SanitizeMessage(message);
		std::cout << std::endl;
	}
}
