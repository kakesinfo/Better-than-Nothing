#pragma once

namespace BetterThanNothing
{
	class CWindow
	{
		GLFWwindow* m_Window = nullptr;

		std::string m_Title{};

		std::uint32_t m_Width = 0;
		std::uint32_t m_Height = 0;

		bool m_fullscreen = false;
		bool m_resizable = true;
		bool m_Resized = false;

	public:
		CWindow() = default;
		explicit CWindow(std::string  title, std::uint32_t width, std::uint32_t height, bool fullscreen = false, bool resizable = false);
		~CWindow();

		[[nodiscard]]
		GLFWwindow* Handle() const;

		static void PollEvents();

		void Show() const;

		[[nodiscard]]
		bool ShouldClose() const;

		[[nodiscard]]
		std::pair<std::uint32_t, std::uint32_t> GetFramebufferSize() const;

		[[nodiscard]]
		bool IsResized() const;
		void SetResized(bool resized);

	protected:
		void SetWidth(std::uint32_t width);
		void SetHeight(std::uint32_t height);

		static void ResizeCallback(GLFWwindow* window, std::int32_t width, std::int32_t height);
		static void KeyCallback(GLFWwindow* window, std::int32_t key, std::int32_t scancode, std::int32_t action, std::int32_t mods);
		static void MouseCursorCallback(GLFWwindow* window, double xpos, double ypos);
		static void MouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
		static void MouseButtonCallback(GLFWwindow* window, std::int32_t button, std::int32_t action, std::int32_t mods);
	};
};
