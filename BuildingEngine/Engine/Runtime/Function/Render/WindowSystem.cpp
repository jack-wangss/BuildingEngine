#include "pch.h"
#include "WindowSystem.h"

namespace BE
{
	WindowSystem::~WindowSystem()
	{
		glfwDestroyWindow(m_Window);
		glfwTerminate();
	}
	void WindowSystem::Initialize(WindowCreateInfo create_info)
	{
		if (!glfwInit())
		{
			//LOG_FATAL(__FUNCTION__, "failed to initialize GLFW");
			return;
		}

		m_Width = create_info.Width;
		m_Height = create_info.Height;

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		m_Window = glfwCreateWindow(create_info.Width, create_info.Height, create_info.Title, nullptr, nullptr);
		if (!m_Window)
		{
			//LOG_FATAL(__FUNCTION__, "failed to create window");
			glfwTerminate();
			return;
		}

		// Setup input callbacks
		glfwSetWindowUserPointer(m_Window, this);
		glfwSetKeyCallback(m_Window, KeyCallback);
		glfwSetCharCallback(m_Window, CharCallback);
		glfwSetCharModsCallback(m_Window, CharModsCallback);
		glfwSetMouseButtonCallback(m_Window, MouseButtonCallback);
		glfwSetCursorPosCallback(m_Window, CursorPosCallback);
		glfwSetCursorEnterCallback(m_Window, CursorEnterCallback);
		glfwSetScrollCallback(m_Window, ScrollCallback);
		glfwSetDropCallback(m_Window, DropCallback);
		glfwSetWindowSizeCallback(m_Window, WindowSizeCallback);
		glfwSetWindowCloseCallback(m_Window, WindowCloseCallback);

		glfwSetInputMode(m_Window, GLFW_RAW_MOUSE_MOTION, GLFW_FALSE);
	}
	void WindowSystem::PollEvents() const
	{
		glfwPollEvents();
	}
	bool WindowSystem::ShouldClose() const
	{
		return glfwWindowShouldClose(m_Window);
	}
	void WindowSystem::SetTitle(const char* title)
	{
		glfwSetWindowTitle(m_Window, title);
	}
	GLFWwindow* WindowSystem::GetWindow() const
	{
		return m_Window;
	}
	std::array<int, 2> WindowSystem::GetWindowSize() const
	{
		return std::array<int, 2>({ m_Width, m_Height });
	}
}
