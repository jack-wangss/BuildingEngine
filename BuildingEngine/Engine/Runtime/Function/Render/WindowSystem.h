#pragma once


namespace BE
{
    struct WindowCreateInfo
    {
        int         Width{ 1280 };
        int         Height{ 720 };
        const char* Title{ "BuildingEngine" };
        bool        IsFFullscreen{ false };
    };
    class WindowSystem
    {
    public:
        WindowSystem() = default;
        ~WindowSystem();
    public:
        void               Initialize(WindowCreateInfo create_info);
        void               PollEvents() const;
        bool               ShouldClose() const;
        void               SetTitle(const char* title);

        typedef std::function<void()>                   OnResetFunc;
        typedef std::function<void(int, int, int, int)> OnKeyFunc;
        typedef std::function<void(unsigned int)>       OnCharFunc;
        typedef std::function<void(int, unsigned int)>  OnCharModsFunc;
        typedef std::function<void(int, int, int)>      OnMouseButtonFunc;
        typedef std::function<void(double, double)>     OnCursorPosFunc;
        typedef std::function<void(int)>                OnCursorEnterFunc;
        typedef std::function<void(double, double)>     OnScrollFunc;
        typedef std::function<void(int, const char**)>  OnDropFunc;
        typedef std::function<void(int, int)>           OnWindowSizeFunc;
        typedef std::function<void()>                   OnWindowCloseFunc;

        void RegisterOnResetFunc(OnResetFunc func) { m_OnResetFunc.push_back(func); }
        void RegisterOnKeyFunc(OnKeyFunc func) { m_OnKeyFunc.push_back(func); }
        void RegisterOnCharFunc(OnCharFunc func) { m_OnCharFunc.push_back(func); }
        void RegisterOnCharModsFunc(OnCharModsFunc func) { m_OnCharModsFunc.push_back(func); }
        void RegisterOnMouseButtonFunc(OnMouseButtonFunc func) { m_OnMouseButtonFunc.push_back(func); }
        void RegisterOnCursorPosFunc(OnCursorPosFunc func) { m_OnCursorPosFunc.push_back(func); }
        void RegisterOnCursorEnterFunc(OnCursorEnterFunc func) { m_OnCursorEnterFunc.push_back(func); }
        void RegisterOnScrollFunc(OnScrollFunc func) { m_OnScrollFunc.push_back(func); }
        void RegisterOnDropFunc(OnDropFunc func) { m_OnDropFunc.push_back(func); }
        void RegisterOnWindowSizeFunc(OnWindowSizeFunc func) { m_OnWindowSizeFunc.push_back(func); }
        void RegisterOnWindowCloseFunc(OnWindowCloseFunc func) { m_OnWindowCloseFunc.push_back(func); }

    protected:
        // window event callbacks
        static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
        {
            WindowSystem* app = (WindowSystem*)glfwGetWindowUserPointer(window);
            if (app)
            {
                app->OnKey(key, scancode, action, mods);
            }
        }
        static void CharCallback(GLFWwindow* window, unsigned int codepoint)
        {
            WindowSystem* app = (WindowSystem*)glfwGetWindowUserPointer(window);
            if (app)
            {
                app->OnChar(codepoint);
            }
        }
        static void CharModsCallback(GLFWwindow* window, unsigned int codepoint, int mods)
        {
            WindowSystem* app = (WindowSystem*)glfwGetWindowUserPointer(window);
            if (app)
            {
                app->OnCharMods(codepoint, mods);
            }
        }
        static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
        {
            WindowSystem* app = (WindowSystem*)glfwGetWindowUserPointer(window);
            if (app)
            {
                app->OnMouseButton(button, action, mods);
            }
        }
        static void CursorPosCallback(GLFWwindow* window, double xpos, double ypos)
        {
            WindowSystem* app = (WindowSystem*)glfwGetWindowUserPointer(window);
            if (app)
            {
                app->OnCursorPos(xpos, ypos);
            }
        }
        static void CursorEnterCallback(GLFWwindow* window, int entered)
        {
            WindowSystem* app = (WindowSystem*)glfwGetWindowUserPointer(window);
            if (app)
            {
                app->OnCursorEnter(entered);
            }
        }
        static void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
        {
            WindowSystem* app = (WindowSystem*)glfwGetWindowUserPointer(window);
            if (app)
            {
                app->OnScroll(xoffset, yoffset);
            }
        }
        static void DropCallback(GLFWwindow* window, int count, const char** paths)
        {
            WindowSystem* app = (WindowSystem*)glfwGetWindowUserPointer(window);
            if (app)
            {
                app->OnDrop(count, paths);
            }
        }
        static void WindowSizeCallback(GLFWwindow* window, int width, int height)
        {
            WindowSystem* app = (WindowSystem*)glfwGetWindowUserPointer(window);
            if (app)
            {
                app->m_Width = width;
                app->m_Height = height;
            }
        }
        static void WindowCloseCallback(GLFWwindow* window) { glfwSetWindowShouldClose(window, true); }

        void OnReset()
        {
            for (auto& func : m_OnResetFunc)
                func();
        }
        void OnKey(int key, int scancode, int action, int mods)
        {
            for (auto& func : m_OnKeyFunc)
                func(key, scancode, action, mods);
        }
        void OnChar(unsigned int codepoint)
        {
            for (auto& func : m_OnCharFunc)
                func(codepoint);
        }
        void OnCharMods(int codepoint, unsigned int mods)
        {
            for (auto& func : m_OnCharModsFunc)
                func(codepoint, mods);
        }
        void OnMouseButton(int button, int action, int mods)
        {
            for (auto& func : m_OnMouseButtonFunc)
                func(button, action, mods);
        }
        void OnCursorPos(double xpos, double ypos)
        {
            for (auto& func : m_OnCursorPosFunc)
                func(xpos, ypos);
        }
        void OnCursorEnter(int entered)
        {
            for (auto& func : m_OnCursorEnterFunc)
                func(entered);
        }
        void OnScroll(double xoffset, double yoffset)
        {
            for (auto& func : m_OnScrollFunc)
                func(xoffset, yoffset);
        }
        void OnDrop(int count, const char** paths)
        {
            for (auto& func : m_OnDropFunc)
                func(count, paths);
        }
        void OnWindowSize(int width, int height)
        {
            for (auto& func : m_OnWindowSizeFunc)
                func(width, height);
        }
    private:
        GLFWwindow* m_Window{ nullptr };
        int         m_Width{ 0 };
        int         m_Height{ 0 };
        bool        m_IsFocusMode{ false };

        std::vector<OnResetFunc>       m_OnResetFunc;
        std::vector<OnKeyFunc>         m_OnKeyFunc;
        std::vector<OnCharFunc>        m_OnCharFunc;
        std::vector<OnCharModsFunc>    m_OnCharModsFunc;
        std::vector<OnMouseButtonFunc> m_OnMouseButtonFunc;
        std::vector<OnCursorPosFunc>   m_OnCursorPosFunc;
        std::vector<OnCursorEnterFunc> m_OnCursorEnterFunc;
        std::vector<OnScrollFunc>      m_OnScrollFunc;
        std::vector<OnDropFunc>        m_OnDropFunc;
        std::vector<OnWindowSizeFunc>  m_OnWindowSizeFunc;
        std::vector<OnWindowCloseFunc> m_OnWindowCloseFunc;
    };
}