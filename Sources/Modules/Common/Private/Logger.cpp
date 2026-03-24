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

#include <Common/Logger.hpp>

#include <chrono>
#include <filesystem>
#include <iomanip>
#include <iostream>

#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
	#undef ERROR
#endif

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

	void Logger::Print_Windows(const uint8_t level, const std::string& levelStr, const std::source_location& location, const std::string_view message)
	{
#ifdef _WIN32
		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		if (hConsole == INVALID_HANDLE_VALUE)
			return;

		WORD color = 7;
		switch (level)
		{
			case DEBUG:   color = 8;  break;
			case INFO:    color = 11; break;
			case SUCCESS: color = 10; break;
			case WARNING: color = 14; break;
			case ERROR:   color = 12; break;
		}

		SetConsoleTextAttribute(hConsole, 7);
		std::cout << std::left << GetTimestamp() << " ";
		SetConsoleTextAttribute(hConsole, color);
		std::cout << std::left << std::setw(10) << levelStr;

		SetConsoleTextAttribute(hConsole, 7);
		std::cout << std::left << std::setw(35) << FormatSourceFile(location);
		std::cout << std::left << SanitizeMessage(message) << std::endl;
#endif
	}

	void Logger::Print_Linux(const uint8_t level, const std::string& levelStr, const std::source_location& location, const std::string_view message)
	{
#ifndef _WIN32
		std::string levelColor;
		switch (level)
		{
			case DEBUG:   levelColor = "\033[90m"; break;
			case INFO:    levelColor = "\033[36m"; break;
			case SUCCESS: levelColor = "\033[32m"; break;
			case WARNING: levelColor = "\033[33m"; break;
			case ERROR:   levelColor = "\033[31m"; break;
		}

		std::cout << "\033[37m" << std::left << GetTimestamp() << " ";
		std::cout << levelColor << std::left << std::setw(10) << levelStr;
		std::cout << "\033[37m" << std::left << std::setw(35) << FormatSourceFile(location);
		std::cout << "\033[0m" << std::left << SanitizeMessage(message);
		std::cout << std::endl;
#endif
	}

	void Logger::Print(const uint8_t level, const std::source_location& location, const std::string_view message)
	{
		if (level < s_currentLogLevel)
			return;

		std::lock_guard lock(s_printMutex);

		std::string levelStr;
		switch (level)
		{
			case DEBUG:
				levelStr = "[DEBUG]";
				break;
			case INFO:
				levelStr = "[INFO]";
				break;
			case SUCCESS:
				levelStr = "[SUCCESS]";
				break;
			case WARNING:
				levelStr = "[WARNING]";
				break;
			case ERROR:
				levelStr = "[ERROR]";
				break;
			default:
				break;
		}

		Print_Windows(level, levelStr, location, message);
		Print_Linux(level, levelStr, location, message);
	}
}
