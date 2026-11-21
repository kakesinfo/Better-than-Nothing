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

#include <Common/AssetPath.hpp>
#include <filesystem>

namespace BetterThanNothing
{
	std::string AssetPath::s_projectRoot;

	static std::filesystem::path StripLeadingParentSegments(const std::filesystem::path& p)
	{
		std::filesystem::path result;
		bool				  stripping = true;
		for (const auto& part : p)
		{
			if (stripping && part == "..")
				continue;
			stripping = false;
			result /= part;
		}
		return result;
	}

	static std::filesystem::path FindProjectRoot()
	{
		std::error_code		  ec;
		std::filesystem::path dir = std::filesystem::current_path(ec);
		for (int depth = 0; depth < 8 && !dir.empty(); ++depth)
		{
			if (std::filesystem::exists(dir / "Shaders", ec) || std::filesystem::exists(dir / ".assets", ec))
				return dir;

			const auto parent = dir.parent_path();
			if (parent == dir)
				break;
			dir = parent;
		}
		return {};
	}

	void AssetPath::SetProjectRoot(const std::string_view rootPath)
	{
		s_projectRoot = std::string(rootPath);
	}

	std::string AssetPath::GetProjectRoot()
	{
		if (s_projectRoot.empty())
		{
			if (const auto detected = FindProjectRoot(); !detected.empty())
				s_projectRoot = detected.string();
		}
		return s_projectRoot;
	}

	std::string AssetPath::Resolve(const std::string_view path)
	{
		if (path.empty())
			return {};

		std::error_code			   ec;
		const std::filesystem::path raw(path);

		// 1. Direct hit (absolute or valid relative to CWD)
		if (std::filesystem::exists(raw, ec))
			return std::filesystem::weakly_canonical(raw, ec).string();

		// 2. Try against detected/configured project root (also handles legacy "../../../" prefixes)
		const std::string rootStr = GetProjectRoot();
		if (!rootStr.empty())
		{
			const std::filesystem::path root(rootStr);
			const std::filesystem::path candidate1 = root / raw;
			if (std::filesystem::exists(candidate1, ec))
				return std::filesystem::weakly_canonical(candidate1, ec).string();

			const std::filesystem::path stripped = StripLeadingParentSegments(raw);
			if (!stripped.empty())
			{
				const std::filesystem::path candidate2 = root / stripped;
				if (std::filesystem::exists(candidate2, ec))
					return std::filesystem::weakly_canonical(candidate2, ec).string();
			}
		}

		// 3. Fallback to original path so error messages remain informative
		return std::string(path);
	}
}
