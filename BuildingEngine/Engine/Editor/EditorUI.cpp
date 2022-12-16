#include "pch.h"
#include "EditorUI.h"

#include "Runtime/Function/Global/RuntimeGlobalContext.h"


namespace BE
{
	EditorUI::EditorUI()
	{
	}
	void EditorUI::Initialize(WindowUIInitInfo info)
	{
        //std::shared_ptr<ConfigManager> config_manager =g_RuntimeGlobalContext.m_RenderSystem;
        //ASSERT(config_manager);

        //// create imgui context
        //IMGUI_CHECKVERSION();
        //ImGui::CreateContext();

        //// set ui content scale
        //float x_scale, y_scale;
        //glfwGetWindowContentScale(init_info.window_system->getWindow(), &x_scale, &y_scale);
        //float content_scale = fmaxf(1.0f, fmaxf(x_scale, y_scale));
        //windowContentScaleUpdate(content_scale);
        //glfwSetWindowContentScaleCallback(init_info.window_system->getWindow(), windowContentScaleCallback);

        //// load font for imgui
        //ImGuiIO& io = ImGui::GetIO();
        //io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        //io.ConfigDockingAlwaysTabBar = true;
        //io.ConfigWindowsMoveFromTitleBarOnly = true;
        //io.Fonts->AddFontFromFileTTF(
        //    config_manager->getEditorFontPath().generic_string().data(), content_scale * 16, nullptr, nullptr);
        //io.Fonts->Build();

        //ImGuiStyle& style = ImGui::GetStyle();
        //style.WindowPadding = ImVec2(1.0, 0);
        //style.FramePadding = ImVec2(14.0, 2.0f);
        //style.ChildBorderSize = 0.0f;
        //style.FrameRounding = 5.0f;
        //style.FrameBorderSize = 1.5f;

        //// set imgui color style
        //setUIColorStyle();

        //// setup window icon
        //GLFWimage   window_icon[2];
        //std::string big_icon_path_string = config_manager->getEditorBigIconPath().generic_string();
        //std::string small_icon_path_string = config_manager->getEditorSmallIconPath().generic_string();
        //window_icon[0].pixels =
        //    stbi_load(big_icon_path_string.data(), &window_icon[0].width, &window_icon[0].height, 0, 4);
        //window_icon[1].pixels =
        //    stbi_load(small_icon_path_string.data(), &window_icon[1].width, &window_icon[1].height, 0, 4);
        //glfwSetWindowIcon(init_info.window_system->getWindow(), 2, window_icon);
        //stbi_image_free(window_icon[0].pixels);
        //stbi_image_free(window_icon[1].pixels);

        //// initialize imgui vulkan render backend
        //init_info.render_system->initializeUIRenderBackend(this);
	}
	void EditorUI::PreRender()
	{
        ShowEditorUI();
	}
    void EditorUI::ShowEditorUI()
    {
    }
}
