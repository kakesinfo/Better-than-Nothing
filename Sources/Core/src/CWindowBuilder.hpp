//
// Created by acoezard on 11/5/24.
//

#pragma once

#include "BetterThanNothing.hpp"

namespace BetterThanNothing
{
	class CWindowBuilderError : public std::runtime_error
	{
	public:
		explicit CWindowBuilderError(const std::string& message)
			: std::runtime_error(message) {}
	};

	struct WindowBuilderParams
	{
		std::string title;
		uint32 width = 0;
		uint32 height = 0;
		bool fullscreen = false;
		bool resizable = false;
	};

	class CWindowBuilder
	{
	private:
		WindowBuilderParams m_windowParams{};

	public:
		CWindowBuilder() = default;
		CWindowBuilder(const std::string& title, uint32 width, uint32 height)
		{
			m_windowParams.title = title;
			m_windowParams.width = width;
			m_windowParams.height = height;
		}

		~CWindowBuilder() = default;

		CWindowBuilder& SetTitle(const std::string& title)
		{
			m_windowParams.title = title;
			return *this;
		}

		CWindowBuilder& SetSize(uint32 width, uint32 height)
		{
			m_windowParams.width = width;
			m_windowParams.height = height;
			return *this;
		}

		CWindowBuilder& SeFullscreen(bool fullscreen)
		{
			m_windowParams.fullscreen = fullscreen;
			return *this;
		}

		CWindowBuilder& SetResizable(bool resizable)
		{
			m_windowParams.resizable = resizable;
			return *this;
		}

		std::unique_ptr<CWindow> Build() const
		{
			if (m_windowParams.title.empty())
				throw CWindowBuilderError("CWindow title is not set");

			if (m_windowParams.width == 0 || m_windowParams.height == 0)
				throw CWindowBuilderError("CWindow size is not set");

			return std::make_unique<CWindow>(
				m_windowParams.title,
				m_windowParams.width,
				m_windowParams.height,
				m_windowParams.fullscreen,
				m_windowParams.resizable
			);
		}
	};
} // BetterThanNothing