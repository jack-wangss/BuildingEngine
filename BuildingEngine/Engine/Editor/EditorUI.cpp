#include "pch.h"
#include "EditorUI.h"

#include "Runtime/Function/Render/WindowSystem.h"
#include "Runtime/Function/Render/RenderSystem.h"
#include "Runtime/Function/Global/RuntimeGlobalContext.h"
#include "Editor/EditorGlobalContext.h"

namespace BE
{
	EditorUI::EditorUI()
	{
	}
	void EditorUI::Initialize(WindowUIInitInfo info)
	{
		//std::shared_ptr<ConfigManager> config_manager =g_RuntimeGlobalContext.m_RenderSystem;
		//ASSERT(config_manager);

		// create imgui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		// set ui content scale
	  /*  float x_scale, y_scale;
		glfwGetWindowContentScale(info.WindowSystem->GetWindow(), &x_scale, &y_scale);
		float content_scale = fmaxf(1.0f, fmaxf(x_scale, y_scale));
		windowContentScaleUpdate(content_scale);
		glfwSetWindowContentScaleCallback(info.WindowSystem->GetWindow(), windowContentScaleCallback);*/

		// load font for imgui
		ImGuiIO& io = ImGui::GetIO();

		//io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		//io.ConfigDockingAlwaysTabBar = true;
		//io.ConfigWindowsMoveFromTitleBarOnly = true;
		//io.Fonts->AddFontFromFileTTF(
		//    config_manager->getEditorFontPath().generic_string().data(), content_scale * 16, nullptr, nullptr);
		io.Fonts->Build();

		ImGuiStyle& style = ImGui::GetStyle();
		style.WindowPadding = ImVec2(1.0, 0);
		style.FramePadding = ImVec2(14.0, 2.0f);
		style.ChildBorderSize = 0.0f;
		style.FrameRounding = 5.0f;
		style.FrameBorderSize = 1.5f;

		// set imgui color style
		ImGuiStyle* styleP = &ImGui::GetStyle();
		ImVec4* colors = styleP->Colors;
		colors[ImGuiCol_Text] = ImVec4(0.4745f, 0.4745f, 0.4745f, 1.00f);
		colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
		colors[ImGuiCol_WindowBg] = ImVec4(0.0078f, 0.0078f, 0.0078f, 1.00f);
		colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
		colors[ImGuiCol_Border] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
		colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_FrameBg] = ImVec4(0.047f, 0.047f, 0.047f, 0.5411f);
		colors[ImGuiCol_FrameBgHovered] = ImVec4(0.196f, 0.196f, 0.196f, 0.40f);
		colors[ImGuiCol_FrameBgActive] = ImVec4(0.294f, 0.294f, 0.294f, 0.67f);
		colors[ImGuiCol_TitleBg] = ImVec4(0.0039f, 0.0039f, 0.0039f, 1.00f);
		colors[ImGuiCol_TitleBgActive] = ImVec4(0.0039f, 0.0039f, 0.0039f, 1.00f);
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.50f);
		colors[ImGuiCol_MenuBarBg] = ImVec4(0.0f, 0.0f, 0.0f, 1.00f);
		colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
		colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
		colors[ImGuiCol_CheckMark] = ImVec4(93.0f / 255.0f, 10.0f / 255.0f, 66.0f / 255.0f, 1.00f);
		colors[ImGuiCol_SliderGrab] = colors[ImGuiCol_CheckMark];
		colors[ImGuiCol_SliderGrabActive] = ImVec4(0.3647f, 0.0392f, 0.2588f, 0.50f);
		colors[ImGuiCol_Button] = ImVec4(0.0117f, 0.0117f, 0.0117f, 1.00f);
		colors[ImGuiCol_ButtonHovered] = ImVec4(0.0235f, 0.0235f, 0.0235f, 1.00f);
		colors[ImGuiCol_ButtonActive] = ImVec4(0.0353f, 0.0196f, 0.0235f, 1.00f);
		colors[ImGuiCol_Header] = ImVec4(0.1137f, 0.0235f, 0.0745f, 0.588f);
		colors[ImGuiCol_HeaderHovered] = ImVec4(5.0f / 255.0f, 5.0f / 255.0f, 5.0f / 255.0f, 1.00f);
		colors[ImGuiCol_HeaderActive] = ImVec4(0.0f, 0.0f, 0.0f, 1.00f);
		colors[ImGuiCol_Separator] = ImVec4(0.0f, 0.0f, 0.0f, 0.50f);
		colors[ImGuiCol_SeparatorHovered] = ImVec4(45.0f / 255.0f, 7.0f / 255.0f, 26.0f / 255.0f, 1.00f);
		colors[ImGuiCol_SeparatorActive] = ImVec4(45.0f / 255.0f, 7.0f / 255.0f, 26.0f / 255.0f, 1.00f);
		colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.20f);
		colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
		colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
		colors[ImGuiCol_Tab] = ImVec4(6.0f / 255.0f, 6.0f / 255.0f, 8.0f / 255.0f, 1.00f);
		colors[ImGuiCol_TabHovered] = ImVec4(45.0f / 255.0f, 7.0f / 255.0f, 26.0f / 255.0f, 150.0f / 255.0f);
		colors[ImGuiCol_TabActive] = ImVec4(47.0f / 255.0f, 6.0f / 255.0f, 29.0f / 255.0f, 1.0f);
		colors[ImGuiCol_TabUnfocused] = ImVec4(45.0f / 255.0f, 7.0f / 255.0f, 26.0f / 255.0f, 25.0f / 255.0f);
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4(6.0f / 255.0f, 6.0f / 255.0f, 8.0f / 255.0f, 200.0f / 255.0f);
		colors[ImGuiCol_DockingPreview] = ImVec4(47.0f / 255.0f, 6.0f / 255.0f, 29.0f / 255.0f, 0.7f);
		colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.20f, 0.20f, 0.20f, 0.00f);
		colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
		colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
		colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
		colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
		colors[ImGuiCol_TableHeaderBg] = ImVec4(0.0f, 0.0f, 0.0f, 1.00f);
		colors[ImGuiCol_TableBorderStrong] = ImVec4(2.0f / 255.0f, 2.0f / 255.0f, 2.0f / 255.0f, 1.0f);
		colors[ImGuiCol_TableBorderLight] = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);
		colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
		colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
		colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
		colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
		colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
		colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);

		// setup window icon
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

		// initialize imgui vulkan render backend
		info.RenderSystem->InitializeUIRenderBackend(this);
	}
	void EditorUI::PreRender()
	{
		ShowEditorUI();
	}
	void EditorUI::ShowEditorUI()
	{
		ShowEditorMenu(&m_EditorMenuWindowOpen);
		ShowEditorWorldObjectsWindow(&m_AssetWindowOpen);
		ShowEditorGameWindow(&m_GameEngineWindowOpen);
		ShowEditorFileContentWindow(&m_FileContentWindowOpen);
		ShowEditorDetailWindow(&m_DetailWindowOpen);
	}
	void EditorUI::ShowEditorMenu(bool* isShow)
	{
		ImGuiDockNodeFlags dock_flags = ImGuiDockNodeFlags_DockSpace;
		ImGuiWindowFlags   window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground |
			ImGuiConfigFlags_NoMouseCursorChange | ImGuiWindowFlags_NoBringToFrontOnFocus;

		const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(main_viewport->WorkPos, ImGuiCond_Always);
		std::array<int, 2> window_size = g_EditorGlobalContext.m_WindowSystem->GetWindowSize();
		ImGui::SetNextWindowSize(ImVec2((float)window_size[0], (float)window_size[1]), ImGuiCond_Always);

		ImGui::SetNextWindowViewport(main_viewport->ID);

		ImGui::Begin("Editor menu", isShow, window_flags);

		ImGuiID main_docking_id = ImGui::GetID("Main Docking");
		if (ImGui::DockBuilderGetNode(main_docking_id) == nullptr)
		{
			ImGui::DockBuilderRemoveNode(main_docking_id);

			ImGui::DockBuilderAddNode(main_docking_id, dock_flags);
			ImGui::DockBuilderSetNodePos(main_docking_id,
				ImVec2(main_viewport->WorkPos.x, main_viewport->WorkPos.y + 18.0f));
			ImGui::DockBuilderSetNodeSize(main_docking_id,
				ImVec2((float)window_size[0], (float)window_size[1] - 18.0f));

			ImGuiID center = main_docking_id;
			ImGuiID left;
			ImGuiID right = ImGui::DockBuilderSplitNode(center, ImGuiDir_Right, 0.25f, nullptr, &left);

			ImGuiID left_other;
			ImGuiID left_file_content = ImGui::DockBuilderSplitNode(left, ImGuiDir_Down, 0.30f, nullptr, &left_other);

			ImGuiID left_game_engine;
			ImGuiID left_asset =
				ImGui::DockBuilderSplitNode(left_other, ImGuiDir_Left, 0.30f, nullptr, &left_game_engine);

			ImGui::DockBuilderDockWindow("World Objects", left_asset);
			ImGui::DockBuilderDockWindow("Components Details", right);
			ImGui::DockBuilderDockWindow("File Content", left_file_content);
			ImGui::DockBuilderDockWindow("Game Engine", left_game_engine);

			ImGui::DockBuilderFinish(main_docking_id);
		}

		ImGui::DockSpace(main_docking_id);

		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("Menu"))
			{
				if (ImGui::MenuItem("Reload Current Level"))
				{
					//g_runtime_global_context.m_world_manager->reloadCurrentLevel();
					//g_runtime_global_context.m_render_system->clearForLevelReloading();
					//g_editor_global_context.m_scene_manager->onGObjectSelected(k_invalid_gobject_id);
				}
				if (ImGui::MenuItem("Save Current Level"))
				{
					//g_runtime_global_context.m_world_manager->saveCurrentLevel();
				}
				if (ImGui::MenuItem("Exit"))
				{
					//g_editor_global_context.m_engine_runtime->shutdownEngine();
					exit(0);
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Window"))
			{
				ImGui::MenuItem("World Objects", nullptr, &m_AssetWindowOpen);
				ImGui::MenuItem("Game", nullptr, &m_GameEngineWindowOpen);
				ImGui::MenuItem("File Content", nullptr, &m_FileContentWindowOpen);
				ImGui::MenuItem("Detail", nullptr, &m_DetailWindowOpen);
				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}

		ImGui::End();
	}
	void EditorUI::ShowEditorWorldObjectsWindow(bool* isShow)
	{
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_None;

		const ImGuiViewport* main_viewport = ImGui::GetMainViewport();

		if (!*isShow)
			return;

		if (!ImGui::Begin("World Objects", isShow, window_flags))
		{
			ImGui::End();
			return;
		}

		//std::shared_ptr<Level> current_active_level =
		//    g_runtime_global_context.m_world_manager->getCurrentActiveLevel().lock();
		//if (current_active_level == nullptr)
		//    return;

		//const LevelObjectsMap& all_gobjects = current_active_level->getAllGObjects();
		//for (auto& id_object_pair : all_gobjects)
		//{
		//    const GObjectID          object_id = id_object_pair.first;
		//    std::shared_ptr<GObject> object = id_object_pair.second;
		//    const std::string        name = object->getName();
		//    if (name.size() > 0)
		//    {
		//        if (ImGui::Selectable(name.c_str(),
		//            g_editor_global_context.m_scene_manager->getSelectedObjectID() == object_id))
		//        {
		//            if (g_editor_global_context.m_scene_manager->getSelectedObjectID() != object_id)
		//            {
		//                g_editor_global_context.m_scene_manager->onGObjectSelected(object_id);
		//            }
		//            else
		//            {
		//                g_editor_global_context.m_scene_manager->onGObjectSelected(k_invalid_gobject_id);
		//            }
		//            break;
		//        }
		//    }
		//}
		ImGui::End();
	}
	void EditorUI::ShowEditorGameWindow(bool* isShow)
	{
		ImGuiIO& io = ImGui::GetIO();
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_MenuBar;

		const ImGuiViewport* main_viewport = ImGui::GetMainViewport();

		if (!*isShow)
			return;

		if (!ImGui::Begin("Game Engine", isShow, window_flags))
		{
			ImGui::End();
			return;
		}

		static bool trans_button_ckecked = false;
		static bool rotate_button_ckecked = false;
		static bool scale_button_ckecked = false;

		//switch (g_editor_global_context.m_scene_manager->getEditorAxisMode())
		//{
		//case EditorAxisMode::TranslateMode:
		//    trans_button_ckecked = true;
		//    rotate_button_ckecked = false;
		//    scale_button_ckecked = false;
		//    break;
		//case EditorAxisMode::RotateMode:
		//    trans_button_ckecked = false;
		//    rotate_button_ckecked = true;
		//    scale_button_ckecked = false;
		//    break;
		//case EditorAxisMode::ScaleMode:
		//    trans_button_ckecked = false;
		//    rotate_button_ckecked = false;
		//    scale_button_ckecked = true;
		//    break;
		//default:
		//    break;
		//}

		if (ImGui::BeginMenuBar())
		{
			ImGui::Indent(10.f);
			//drawAxisToggleButton("Trans", trans_button_ckecked, (int)EditorAxisMode::TranslateMode);
			ImGui::Unindent();

			ImGui::SameLine();

			//drawAxisToggleButton("Rotate", rotate_button_ckecked, (int)EditorAxisMode::RotateMode);

			ImGui::SameLine();

			//drawAxisToggleButton("Scale", scale_button_ckecked, (int)EditorAxisMode::ScaleMode);

			ImGui::SameLine();

			float indent_val = 0.0f;

#if defined(__GNUC__) && defined(__MACH__)
			float indent_scale = 1.0f;
#else // Not tested on Linux
			float x_scale, y_scale;

			glfwGetWindowContentScale(g_EditorGlobalContext.m_WindowSystem->GetWindow(), &x_scale, &y_scale);
			float indent_scale = fmaxf(1.0f, fmaxf(x_scale, y_scale));
#endif
			//indent_val = g_editor_global_context.m_input_manager->getEngineWindowSize().x - 100.0f * indent_scale;

			ImGui::Indent(indent_val);
			//if (g_is_editor_mode)
			//{
			//    ImGui::PushID("Editor Mode");
			//    if (ImGui::Button("Editor Mode"))
			//    {
			//        g_is_editor_mode = false;
			//        g_editor_global_context.m_scene_manager->drawSelectedEntityAxis();
			//        g_editor_global_context.m_input_manager->resetEditorCommand();
			//        g_editor_global_context.m_window_system->setFocusMode(true);
			//    }
			//    ImGui::PopID();
			//}
			//else
			//{
			//    if (ImGui::Button("Game Mode"))
			//    {
			//        g_is_editor_mode = true;
			//        g_editor_global_context.m_scene_manager->drawSelectedEntityAxis();
			//        g_runtime_global_context.m_input_system->resetGameCommand();
			//        g_editor_global_context.m_render_system->getRenderCamera()->setMainViewMatrix(
			//            g_editor_global_context.m_scene_manager->getEditorCamera()->getViewMatrix());
			//    }
			//}

			ImGui::Unindent();
			ImGui::EndMenuBar();
		}

		//if (!g_is_editor_mode)
		//{
		//    ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Press Left Alt key to display the mouse cursor!");
		//}
		//else
		//{
		//    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f),
		//        "Current editor camera move speed: [%f]",
		//        g_editor_global_context.m_input_manager->getCameraSpeed());
		//}

		// GetWindowPos() ----->  X--------------------------------------------O
		//                        |                                            |
		//                        |                                            |
		// menu_bar_rect.Min -->  X--------------------------------------------O
		//                        |    It is the menu bar window.              |
		//                        |                                            |
		//                        O--------------------------------------------X  <-- menu_bar_rect.Max
		//                        |                                            |
		//                        |     It is the render target window.        |
		//                        |                                            |
		//                        O--------------------------------------------O

		//Vector2 render_target_window_pos = { 0.0f, 0.0f };
		//Vector2 render_target_window_size = { 0.0f, 0.0f };
		ImVec2 render_target_window_pos = { 0.0f, 0.0f };
		ImVec2 render_target_window_size = { 0.0f, 0.0f };

		auto menu_bar_rect = ImGui::GetCurrentWindow()->MenuBarRect();

		render_target_window_pos.x = ImGui::GetWindowPos().x;
		render_target_window_pos.y = menu_bar_rect.Max.y;
		render_target_window_size.x = ImGui::GetWindowSize().x;
		render_target_window_size.y = (ImGui::GetWindowSize().y + ImGui::GetWindowPos().y) - menu_bar_rect.Max.y; // coord of right bottom point of full window minus coord of right bottom point of menu bar window.

		// if (new_window_pos != m_engine_window_pos || new_window_size != m_engine_window_size)
//        {
//#if defined(__MACH__)
//            // The dpi_scale is not reactive to DPI changes or monitor switching, it might be a bug from ImGui.
//            // Return value from ImGui::GetMainViewport()->DpiScal is always the same as first frame.
//            // glfwGetMonitorContentScale and glfwSetWindowContentScaleCallback are more adaptive.
//            float dpi_scale = main_viewport->DpiScale;
//            g_runtime_global_context.m_render_system->updateEngineContentViewport(render_target_window_pos.x * dpi_scale,
//                render_target_window_pos.y * dpi_scale,
//                render_target_window_size.x * dpi_scale,
//                render_target_window_size.y * dpi_scale);
//#else
//            g_runtime_global_context.m_render_system->updateEngineContentViewport(
//                render_target_window_pos.x, render_target_window_pos.y, render_target_window_size.x, render_target_window_size.y);
//#endif
//            g_editor_global_context.m_input_manager->setEngineWindowPos(render_target_window_pos);
//            g_editor_global_context.m_input_manager->setEngineWindowSize(render_target_window_size);
//        }

		ImGui::End();
	}
	void EditorUI::ShowEditorFileContentWindow(bool* isShow)
	{
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_None;

		const ImGuiViewport* main_viewport = ImGui::GetMainViewport();

		if (!*isShow)
			return;

		if (!ImGui::Begin("File Content", isShow, window_flags))
		{
			ImGui::End();
			return;
		}

		static ImGuiTableFlags flags = ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersOuterH |
			ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg |
			ImGuiTableFlags_NoBordersInBody;

		//if (ImGui::BeginTable("File Content", 2, flags))
		//{
		//    ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_NoHide);
		//    ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed);
		//    ImGui::TableHeadersRow();

		//    auto current_time = std::chrono::steady_clock::now();
		//    if (current_time - m_last_file_tree_update > std::chrono::seconds(1))
		//    {
		//        m_editor_file_service.buildEngineFileTree();
		//        m_last_file_tree_update = current_time;
		//    }
		//    m_last_file_tree_update = current_time;

		//    EditorFileNode* editor_root_node = m_editor_file_service.getEditorRootNode();
		//    buildEditorFileAssetsUITree(editor_root_node);
		//    ImGui::EndTable();
		//}

		// file image list

		ImGui::End();
	}
	void EditorUI::ShowEditorDetailWindow(bool* isShow)
	{
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_None;

		const ImGuiViewport* main_viewport = ImGui::GetMainViewport();

		if (!*isShow)
			return;

		if (!ImGui::Begin("Components Details", isShow, window_flags))
		{
			ImGui::End();
			return;
		}

		//std::shared_ptr<GObject> selected_object = g_editor_global_context.m_scene_manager->getSelectedGObject().lock();
		//if (selected_object == nullptr)
		//{
		//    ImGui::End();
		//    return;
		//}

		//const std::string& name = selected_object->getName();
		//static char        cname[128];
		//memset(cname, 0, 128);
		//memcpy(cname, name.c_str(), name.size());

		ImGui::Text("Name");
		ImGui::SameLine();
		//ImGui::InputText("##Name", cname, IM_ARRAYSIZE(cname), ImGuiInputTextFlags_ReadOnly);

		static ImGuiTableFlags flags = ImGuiTableFlags_Resizable | ImGuiTableFlags_NoSavedSettings;
		//auto&& selected_object_components = selected_object->getComponents();
		//for (auto component_ptr : selected_object_components)
		//{
		//    m_editor_ui_creator["TreeNodePush"](("<" + component_ptr.getTypeName() + ">").c_str(), nullptr);
		//    auto object_instance = Reflection::ReflectionInstance(
		//        Piccolo::Reflection::TypeMeta::newMetaFromName(component_ptr.getTypeName().c_str()),
		//        component_ptr.operator->());
		//    createClassUI(object_instance);
		//    m_editor_ui_creator["TreeNodePop"](("<" + component_ptr.getTypeName() + ">").c_str(), nullptr);
		//}
		ImGui::End();
	}
}
