#include "pch.h"
#include "UIPass.h"

#include "Runtime/Function/UI/WindowUI.h"

#include "Runtime/Function/Render/Rhi/Vulkan/VulkanRhi.h"

namespace BE
{
	void UIPass::Initialize(const RenderPassInitInfo* initInfo)
	{
		RenderPass::Initialize(nullptr);

        m_Framebuffer.RenderPass = static_cast<const UIPassInitInfo*>(initInfo)->RenderPass;
	}
	void UIPass::InitializeUIRenderBackend(WindowUI* windowUI)
	{
		m_WindowUI = windowUI;

        ImGui_ImplGlfw_InitForVulkan(m_VulkanRhi->m_Window, true);
        ImGui_ImplVulkan_InitInfo initInfo = {};
        initInfo.Instance                  = m_VulkanRhi->m_Instance;
        initInfo.PhysicalDevice            = m_VulkanRhi->m_PhysicalDevice;
        initInfo.Device                    = m_VulkanRhi->m_Device;
        initInfo.QueueFamily               = m_VulkanRhi->m_QueueIndices.m_GraphicsFamily.value();
        initInfo.Queue                     = m_VulkanRhi->m_GraphicsQueue;
        initInfo.DescriptorPool            = m_VulkanRhi->m_DescriptorPool;
        initInfo.Subpass                   = _main_camera_subpass_ui;

        // may be different from the real swapchain image count
        // see ImGui_ImplVulkanH_GetMinImageCountFromPresentMode
        initInfo.MinImageCount = 3;
        initInfo.ImageCount = 3;
        ImGui_ImplVulkan_Init(&initInfo, m_Framebuffer.RenderPass);
	}
	void UIPass::Draw()
	{
        if (m_WindowUI)
        {
            ImGui_ImplVulkan_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            ImGui::ShowDemoWindow();
            m_WindowUI->PreRender();

            //if (m_vulkan_rhi->isDebugLabelEnabled())
            //{
            //    VkDebugUtilsLabelEXT label_info = {
            //        VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT, nullptr, "ImGUI", {1.0f, 1.0f, 1.0f, 1.0f} };
            //    m_vulkan_rhi->m_VKCmd_begin_debug_utils_label_ext(m_vulkan_rhi->m_current_command_buffer, &label_info);
            //}

            ImGui::Render();

            //ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), m_vulkan_rhi->m_current_command_buffer);

            //if (m_vulkan_rhi->isDebugLabelEnabled())
            //{
            //    m_vulkan_rhi->m_VKCmd_end_debug_utils_label_ext(m_vulkan_rhi->m_current_command_buffer);
            //}
        }
	}
	void UIPass::UploadFonts()
	{
	}
}
