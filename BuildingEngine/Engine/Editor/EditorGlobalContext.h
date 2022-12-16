#pragma once

namespace BE
{
    struct EditorGlobalContextInitInfo
    {
        class WindowSystem* WindowSystem;
        class RenderSystem* RenderSystem;
        class Engine* Engine;
    };

    class EditorGlobalContext
    {
    public:
        class EditorSceneManager* m_SceneManager{ nullptr };
        class EditorInputManager* m_InputManager{ nullptr };
        class RenderSystem* m_RenderSystem{ nullptr };
        class WindowSystem* m_WindowSystem{ nullptr };
        class Engine* m_Engine{ nullptr };

    public:
        void Initialize(const EditorGlobalContextInitInfo& info);
        void Clear();
    };

    extern EditorGlobalContext g_EditorGlobalContext;
}