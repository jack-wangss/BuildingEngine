#include "pch.h"
#include "MainCameraPass.h"

#include "Runtime/Function/Render/Rhi/Vulkan/VulkanUtil.h"
#include "Runtime/Function/Render/Rhi/Vulkan/VulkanRhi.h"

#include "Runtime/Function/Render/RenderMesh.h"
#include "Runtime/Function/Render/RenderCommon.h"
#include "Runtime/Function/Render/RenderResource.h"

#include <Shader/Generated/mesh_vert.h>
#include <Shader/Generated/mesh_gbuffer_frag.h>
#include <Shader/Generated/deferred_lighting_vert.h>
#include <Shader/Generated/deferred_lighting_frag.h>
#include <Shader/Generated/mesh_frag.h>
#include <Shader/Generated/skybox_vert.h>
#include <Shader/Generated/skybox_frag.h>
#include <Shader/Generated/axis_vert.h>
#include <Shader/Generated/axis_frag.h>

namespace BE
{
	void MainCameraPass::Initialize(const RenderPassInitInfo* initInfo)
	{
		RenderPass::Initialize(nullptr);

        const MainCameraPassInitInfo* _init_info = static_cast<const MainCameraPassInitInfo*>(initInfo);
        m_EnableFxaa = _init_info->EnableFxaa;

		SetupAttachments();
		SetupRenderPass(); 
		SetupDescriptorSetLayout();
		SetupPipelines();
		//SetupDescriptorSet();
		//SetupFramebufferDescriptorSet();
		//SetupSwapchainFramebuffers();
	}
	void MainCameraPass::Draw(UIPass& uiPass)
	{
		uiPass.Draw();
	}
	void MainCameraPass::SetupAttachments()
	{
        m_Framebuffer.Attachments.resize(_main_camera_pass_custom_attachment_count +
            _main_camera_pass_post_process_attachment_count);

        m_Framebuffer.Attachments[_main_camera_pass_gbuffer_a].Format = VK_FORMAT_R8G8B8A8_UNORM;
        m_Framebuffer.Attachments[_main_camera_pass_gbuffer_b].Format = VK_FORMAT_R8G8B8A8_UNORM;
        m_Framebuffer.Attachments[_main_camera_pass_gbuffer_c].Format = VK_FORMAT_R8G8B8A8_SRGB;
        m_Framebuffer.Attachments[_main_camera_pass_backup_buffer_odd].Format = VK_FORMAT_R16G16B16A16_SFLOAT;
        m_Framebuffer.Attachments[_main_camera_pass_backup_buffer_even].Format = VK_FORMAT_R16G16B16A16_SFLOAT;

        for (int buffer_index = 0; buffer_index < _main_camera_pass_custom_attachment_count; ++buffer_index)
        {
            if (buffer_index == _main_camera_pass_gbuffer_a)
            {
                VulkanUtil::CreateImage(m_VulkanRhi->m_PhysicalDevice,
                    m_VulkanRhi->m_Device,
                    m_VulkanRhi->m_SwapchainExtent.width,
                    m_VulkanRhi->m_SwapchainExtent.height,
                    m_Framebuffer.Attachments[_main_camera_pass_gbuffer_a].Format,
                    VK_IMAGE_TILING_OPTIMAL,
                    VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                    VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                    m_Framebuffer.Attachments[_main_camera_pass_gbuffer_a].Image,
                    m_Framebuffer.Attachments[_main_camera_pass_gbuffer_a].Mem,
                    0,
                    1,
                    1);
            }
            else
            {
                VulkanUtil::CreateImage(m_VulkanRhi->m_PhysicalDevice,
                    m_VulkanRhi->m_Device,
                    m_VulkanRhi->m_SwapchainExtent.width,
                    m_VulkanRhi->m_SwapchainExtent.height,
                    m_Framebuffer.Attachments[buffer_index].Format,
                    VK_IMAGE_TILING_OPTIMAL,
                    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT |
                    VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT,
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                    m_Framebuffer.Attachments[buffer_index].Image,
                    m_Framebuffer.Attachments[buffer_index].Mem,
                    0,
                    1,
                    1);
            }

            m_Framebuffer.Attachments[buffer_index].View =
                VulkanUtil::CreateImageView(m_VulkanRhi->m_Device,
                    m_Framebuffer.Attachments[buffer_index].Image,
                    m_Framebuffer.Attachments[buffer_index].Format,
                    VK_IMAGE_ASPECT_COLOR_BIT,
                    VK_IMAGE_VIEW_TYPE_2D,
                    1,
                    1);
        }

        m_Framebuffer.Attachments[_main_camera_pass_post_process_buffer_odd].Format = VK_FORMAT_R16G16B16A16_SFLOAT;
        m_Framebuffer.Attachments[_main_camera_pass_post_process_buffer_even].Format = VK_FORMAT_R16G16B16A16_SFLOAT;
        for (int attachment_index = _main_camera_pass_custom_attachment_count;
            attachment_index <
            _main_camera_pass_custom_attachment_count + _main_camera_pass_post_process_attachment_count;
            ++attachment_index)
        {
            VulkanUtil::CreateImage(m_VulkanRhi->m_PhysicalDevice,
                m_VulkanRhi->m_Device,
                m_VulkanRhi->m_SwapchainExtent.width,
                m_VulkanRhi->m_SwapchainExtent.height,
                m_Framebuffer.Attachments[attachment_index].Format,
                VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT |
                VK_IMAGE_USAGE_SAMPLED_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                m_Framebuffer.Attachments[attachment_index].Image,
                m_Framebuffer.Attachments[attachment_index].Mem,
                0,
                1,
                1);

            m_Framebuffer.Attachments[attachment_index].View =
                VulkanUtil::CreateImageView(m_VulkanRhi->m_Device,
                    m_Framebuffer.Attachments[attachment_index].Image,
                    m_Framebuffer.Attachments[attachment_index].Format,
                    VK_IMAGE_ASPECT_COLOR_BIT,
                    VK_IMAGE_VIEW_TYPE_2D,
                    1,
                    1);
        }
	}
	void MainCameraPass::SetupRenderPass()
	{
        VkAttachmentDescription Attachments[_main_camera_pass_attachment_count] = {};

        VkAttachmentDescription& gbuffer_normal_attachment_description = Attachments[_main_camera_pass_gbuffer_a];
        gbuffer_normal_attachment_description.format = m_Framebuffer.Attachments[_main_camera_pass_gbuffer_a].Format;
        gbuffer_normal_attachment_description.samples = VK_SAMPLE_COUNT_1_BIT;
        gbuffer_normal_attachment_description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        gbuffer_normal_attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        gbuffer_normal_attachment_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        gbuffer_normal_attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        gbuffer_normal_attachment_description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        gbuffer_normal_attachment_description.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkAttachmentDescription& gbuffer_metallic_roughness_shadingmodeid_attachment_description =
            Attachments[_main_camera_pass_gbuffer_b];
        gbuffer_metallic_roughness_shadingmodeid_attachment_description.format =
            m_Framebuffer.Attachments[_main_camera_pass_gbuffer_b].Format;
        gbuffer_metallic_roughness_shadingmodeid_attachment_description.samples = VK_SAMPLE_COUNT_1_BIT;
        gbuffer_metallic_roughness_shadingmodeid_attachment_description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        gbuffer_metallic_roughness_shadingmodeid_attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        gbuffer_metallic_roughness_shadingmodeid_attachment_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        gbuffer_metallic_roughness_shadingmodeid_attachment_description.stencilStoreOp =
            VK_ATTACHMENT_STORE_OP_DONT_CARE;
        gbuffer_metallic_roughness_shadingmodeid_attachment_description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        gbuffer_metallic_roughness_shadingmodeid_attachment_description.finalLayout =
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkAttachmentDescription& gbuffer_albedo_attachment_description = Attachments[_main_camera_pass_gbuffer_c];
        gbuffer_albedo_attachment_description.format = m_Framebuffer.Attachments[_main_camera_pass_gbuffer_c].Format;
        gbuffer_albedo_attachment_description.samples = VK_SAMPLE_COUNT_1_BIT;
        gbuffer_albedo_attachment_description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        gbuffer_albedo_attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        gbuffer_albedo_attachment_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        gbuffer_albedo_attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        gbuffer_albedo_attachment_description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        gbuffer_albedo_attachment_description.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkAttachmentDescription& backup_odd_color_attachment_description =
            Attachments[_main_camera_pass_backup_buffer_odd];
        backup_odd_color_attachment_description.format =
            m_Framebuffer.Attachments[_main_camera_pass_backup_buffer_odd].Format;
        backup_odd_color_attachment_description.samples = VK_SAMPLE_COUNT_1_BIT;
        backup_odd_color_attachment_description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        backup_odd_color_attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        backup_odd_color_attachment_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        backup_odd_color_attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        backup_odd_color_attachment_description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        backup_odd_color_attachment_description.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkAttachmentDescription& backup_even_color_attachment_description =
            Attachments[_main_camera_pass_backup_buffer_even];
        backup_even_color_attachment_description.format =
            m_Framebuffer.Attachments[_main_camera_pass_backup_buffer_even].Format;
        backup_even_color_attachment_description.samples = VK_SAMPLE_COUNT_1_BIT;
        backup_even_color_attachment_description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        backup_even_color_attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        backup_even_color_attachment_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        backup_even_color_attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        backup_even_color_attachment_description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        backup_even_color_attachment_description.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkAttachmentDescription& post_process_odd_color_attachment_description =
            Attachments[_main_camera_pass_post_process_buffer_odd];
        post_process_odd_color_attachment_description.format =
            m_Framebuffer.Attachments[_main_camera_pass_post_process_buffer_odd].Format;
        post_process_odd_color_attachment_description.samples = VK_SAMPLE_COUNT_1_BIT;
        post_process_odd_color_attachment_description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        post_process_odd_color_attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        post_process_odd_color_attachment_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        post_process_odd_color_attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        post_process_odd_color_attachment_description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        post_process_odd_color_attachment_description.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkAttachmentDescription& post_process_even_color_attachment_description =
            Attachments[_main_camera_pass_post_process_buffer_even];
        post_process_even_color_attachment_description.format =
            m_Framebuffer.Attachments[_main_camera_pass_post_process_buffer_even].Format;
        post_process_even_color_attachment_description.samples = VK_SAMPLE_COUNT_1_BIT;
        post_process_even_color_attachment_description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        post_process_even_color_attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        post_process_even_color_attachment_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        post_process_even_color_attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        post_process_even_color_attachment_description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        post_process_even_color_attachment_description.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkAttachmentDescription& depth_attachment_description = Attachments[_main_camera_pass_depth];
        depth_attachment_description.format = m_VulkanRhi->m_DepthImageFormat;
        depth_attachment_description.samples = VK_SAMPLE_COUNT_1_BIT;
        depth_attachment_description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depth_attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        depth_attachment_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depth_attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depth_attachment_description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depth_attachment_description.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentDescription& swapchain_image_attachment_description =
            Attachments[_main_camera_pass_swap_chain_image];
        swapchain_image_attachment_description.format = m_VulkanRhi->m_SwapchainImageFormat;
        swapchain_image_attachment_description.samples = VK_SAMPLE_COUNT_1_BIT;
        swapchain_image_attachment_description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        swapchain_image_attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        swapchain_image_attachment_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        swapchain_image_attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        swapchain_image_attachment_description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        swapchain_image_attachment_description.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkSubpassDescription subpasses[_main_camera_subpass_count] = {};

        VkAttachmentReference base_pass_color_Attachments_reference[3] = {};
        base_pass_color_Attachments_reference[0].attachment = &gbuffer_normal_attachment_description - Attachments;
        base_pass_color_Attachments_reference[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        base_pass_color_Attachments_reference[1].attachment =
            &gbuffer_metallic_roughness_shadingmodeid_attachment_description - Attachments;
        base_pass_color_Attachments_reference[1].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        base_pass_color_Attachments_reference[2].attachment = &gbuffer_albedo_attachment_description - Attachments;
        base_pass_color_Attachments_reference[2].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference base_pass_depth_attachment_reference{};
        base_pass_depth_attachment_reference.attachment = &depth_attachment_description - Attachments;
        base_pass_depth_attachment_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkSubpassDescription& base_pass = subpasses[_main_camera_subpass_basepass];
        base_pass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        base_pass.colorAttachmentCount =
            sizeof(base_pass_color_Attachments_reference) / sizeof(base_pass_color_Attachments_reference[0]);
        base_pass.pColorAttachments = &base_pass_color_Attachments_reference[0];
        base_pass.pDepthStencilAttachment = &base_pass_depth_attachment_reference;
        base_pass.preserveAttachmentCount = 0;
        base_pass.pPreserveAttachments = NULL;

        VkAttachmentReference deferred_lighting_pass_input_Attachments_reference[4] = {};
        deferred_lighting_pass_input_Attachments_reference[0].attachment =
            &gbuffer_normal_attachment_description - Attachments;
        deferred_lighting_pass_input_Attachments_reference[0].layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        deferred_lighting_pass_input_Attachments_reference[1].attachment =
            &gbuffer_metallic_roughness_shadingmodeid_attachment_description - Attachments;
        deferred_lighting_pass_input_Attachments_reference[1].layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        deferred_lighting_pass_input_Attachments_reference[2].attachment =
            &gbuffer_albedo_attachment_description - Attachments;
        deferred_lighting_pass_input_Attachments_reference[2].layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        deferred_lighting_pass_input_Attachments_reference[3].attachment = &depth_attachment_description - Attachments;
        deferred_lighting_pass_input_Attachments_reference[3].layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkAttachmentReference deferred_lighting_pass_color_attachment_reference[1] = {};
        deferred_lighting_pass_color_attachment_reference[0].attachment =
            &backup_odd_color_attachment_description - Attachments;
        deferred_lighting_pass_color_attachment_reference[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription& deferred_lighting_pass = subpasses[_main_camera_subpass_deferred_lighting];
        deferred_lighting_pass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        deferred_lighting_pass.inputAttachmentCount = sizeof(deferred_lighting_pass_input_Attachments_reference) /
            sizeof(deferred_lighting_pass_input_Attachments_reference[0]);
        deferred_lighting_pass.pInputAttachments = &deferred_lighting_pass_input_Attachments_reference[0];
        deferred_lighting_pass.colorAttachmentCount = sizeof(deferred_lighting_pass_color_attachment_reference) /
            sizeof(deferred_lighting_pass_color_attachment_reference[0]);
        deferred_lighting_pass.pColorAttachments = &deferred_lighting_pass_color_attachment_reference[0];
        deferred_lighting_pass.pDepthStencilAttachment = NULL;
        deferred_lighting_pass.preserveAttachmentCount = 0;
        deferred_lighting_pass.pPreserveAttachments = NULL;

        VkAttachmentReference forward_lighting_pass_color_Attachments_reference[1] = {};
        forward_lighting_pass_color_Attachments_reference[0].attachment =
            &backup_odd_color_attachment_description - Attachments;
        forward_lighting_pass_color_Attachments_reference[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference forward_lighting_pass_depth_attachment_reference{};
        forward_lighting_pass_depth_attachment_reference.attachment = &depth_attachment_description - Attachments;
        forward_lighting_pass_depth_attachment_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkSubpassDescription& forward_lighting_pass = subpasses[_main_camera_subpass_forward_lighting];
        forward_lighting_pass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        forward_lighting_pass.inputAttachmentCount = 0U;
        forward_lighting_pass.pInputAttachments = NULL;
        forward_lighting_pass.colorAttachmentCount = sizeof(forward_lighting_pass_color_Attachments_reference) /
            sizeof(forward_lighting_pass_color_Attachments_reference[0]);
        forward_lighting_pass.pColorAttachments = &forward_lighting_pass_color_Attachments_reference[0];
        forward_lighting_pass.pDepthStencilAttachment = &forward_lighting_pass_depth_attachment_reference;
        forward_lighting_pass.preserveAttachmentCount = 0;
        forward_lighting_pass.pPreserveAttachments = NULL;

        VkAttachmentReference tone_mapping_pass_input_attachment_reference{};
        tone_mapping_pass_input_attachment_reference.attachment =
            &backup_odd_color_attachment_description - Attachments;
        tone_mapping_pass_input_attachment_reference.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkAttachmentReference tone_mapping_pass_color_attachment_reference{};
        tone_mapping_pass_color_attachment_reference.attachment =
            &backup_even_color_attachment_description - Attachments;
        tone_mapping_pass_color_attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription& tone_mapping_pass = subpasses[_main_camera_subpass_tone_mapping];
        tone_mapping_pass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        tone_mapping_pass.inputAttachmentCount = 1;
        tone_mapping_pass.pInputAttachments = &tone_mapping_pass_input_attachment_reference;
        tone_mapping_pass.colorAttachmentCount = 1;
        tone_mapping_pass.pColorAttachments = &tone_mapping_pass_color_attachment_reference;
        tone_mapping_pass.pDepthStencilAttachment = NULL;
        tone_mapping_pass.preserveAttachmentCount = 0;
        tone_mapping_pass.pPreserveAttachments = NULL;

        VkAttachmentReference color_grading_pass_input_attachment_reference{};
        color_grading_pass_input_attachment_reference.attachment =
            &backup_even_color_attachment_description - Attachments;
        color_grading_pass_input_attachment_reference.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkAttachmentReference color_grading_pass_color_attachment_reference{};
        if (m_EnableFxaa)
        {
            color_grading_pass_color_attachment_reference.attachment =
                &post_process_odd_color_attachment_description - Attachments;
        }
        else
        {
            color_grading_pass_color_attachment_reference.attachment =
                &backup_odd_color_attachment_description - Attachments;
        }
        color_grading_pass_color_attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription& color_grading_pass = subpasses[_main_camera_subpass_color_grading];
        color_grading_pass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        color_grading_pass.inputAttachmentCount = 1;
        color_grading_pass.pInputAttachments = &color_grading_pass_input_attachment_reference;
        color_grading_pass.colorAttachmentCount = 1;
        color_grading_pass.pColorAttachments = &color_grading_pass_color_attachment_reference;
        color_grading_pass.pDepthStencilAttachment = NULL;
        color_grading_pass.preserveAttachmentCount = 0;
        color_grading_pass.pPreserveAttachments = NULL;

        VkAttachmentReference fxaa_pass_input_attachment_reference{};
        if (m_EnableFxaa)
        {
            fxaa_pass_input_attachment_reference.attachment =
                &post_process_odd_color_attachment_description - Attachments;
        }
        else
        {
            fxaa_pass_input_attachment_reference.attachment = &backup_even_color_attachment_description - Attachments;
        }
        fxaa_pass_input_attachment_reference.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkAttachmentReference fxaa_pass_color_attachment_reference{};
        fxaa_pass_color_attachment_reference.attachment = &backup_odd_color_attachment_description - Attachments;
        fxaa_pass_color_attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription& fxaa_pass = subpasses[_main_camera_subpass_fxaa];
        fxaa_pass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        fxaa_pass.inputAttachmentCount = 1;
        fxaa_pass.pInputAttachments = &fxaa_pass_input_attachment_reference;
        fxaa_pass.colorAttachmentCount = 1;
        fxaa_pass.pColorAttachments = &fxaa_pass_color_attachment_reference;
        fxaa_pass.pDepthStencilAttachment = NULL;
        fxaa_pass.preserveAttachmentCount = 0;
        fxaa_pass.pPreserveAttachments = NULL;

        VkAttachmentReference ui_pass_color_attachment_reference{};
        ui_pass_color_attachment_reference.attachment = &backup_even_color_attachment_description - Attachments;
        ui_pass_color_attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        uint32_t ui_pass_preserve_attachment = &backup_odd_color_attachment_description - Attachments;

        VkSubpassDescription& ui_pass = subpasses[_main_camera_subpass_ui];
        ui_pass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        ui_pass.inputAttachmentCount = 0;
        ui_pass.pInputAttachments = NULL;
        ui_pass.colorAttachmentCount = 1;
        ui_pass.pColorAttachments = &ui_pass_color_attachment_reference;
        ui_pass.pDepthStencilAttachment = NULL;
        ui_pass.preserveAttachmentCount = 1;
        ui_pass.pPreserveAttachments = &ui_pass_preserve_attachment;

        VkAttachmentReference combine_ui_pass_input_Attachments_reference[2] = {};
        combine_ui_pass_input_Attachments_reference[0].attachment =
            &backup_odd_color_attachment_description - Attachments;
        combine_ui_pass_input_Attachments_reference[0].layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        combine_ui_pass_input_Attachments_reference[1].attachment =
            &backup_even_color_attachment_description - Attachments;
        combine_ui_pass_input_Attachments_reference[1].layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkAttachmentReference combine_ui_pass_color_attachment_reference{};
        combine_ui_pass_color_attachment_reference.attachment = &swapchain_image_attachment_description - Attachments;
        combine_ui_pass_color_attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription& combine_ui_pass = subpasses[_main_camera_subpass_combine_ui];
        combine_ui_pass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        combine_ui_pass.inputAttachmentCount = sizeof(combine_ui_pass_input_Attachments_reference) /
            sizeof(combine_ui_pass_input_Attachments_reference[0]);
        combine_ui_pass.pInputAttachments = combine_ui_pass_input_Attachments_reference;
        combine_ui_pass.colorAttachmentCount = 1;
        combine_ui_pass.pColorAttachments = &combine_ui_pass_color_attachment_reference;
        combine_ui_pass.pDepthStencilAttachment = NULL;
        combine_ui_pass.preserveAttachmentCount = 0;
        combine_ui_pass.pPreserveAttachments = NULL;

        VkSubpassDependency dependencies[8] = {};

        VkSubpassDependency& deferred_lighting_pass_depend_on_shadow_map_pass = dependencies[0];
        deferred_lighting_pass_depend_on_shadow_map_pass.srcSubpass = VK_SUBPASS_EXTERNAL;
        deferred_lighting_pass_depend_on_shadow_map_pass.dstSubpass = _main_camera_subpass_deferred_lighting;
        deferred_lighting_pass_depend_on_shadow_map_pass.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        deferred_lighting_pass_depend_on_shadow_map_pass.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        deferred_lighting_pass_depend_on_shadow_map_pass.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        deferred_lighting_pass_depend_on_shadow_map_pass.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        deferred_lighting_pass_depend_on_shadow_map_pass.dependencyFlags = 0; // NOT BY REGION

        VkSubpassDependency& deferred_lighting_pass_depend_on_base_pass = dependencies[1];
        deferred_lighting_pass_depend_on_base_pass.srcSubpass = _main_camera_subpass_basepass;
        deferred_lighting_pass_depend_on_base_pass.dstSubpass = _main_camera_subpass_deferred_lighting;
        deferred_lighting_pass_depend_on_base_pass.srcStageMask =
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        deferred_lighting_pass_depend_on_base_pass.dstStageMask =
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        deferred_lighting_pass_depend_on_base_pass.srcAccessMask =
            VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        deferred_lighting_pass_depend_on_base_pass.dstAccessMask =
            VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
        deferred_lighting_pass_depend_on_base_pass.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        VkSubpassDependency& forward_lighting_pass_depend_on_deferred_lighting_pass = dependencies[2];
        forward_lighting_pass_depend_on_deferred_lighting_pass.srcSubpass = _main_camera_subpass_deferred_lighting;
        forward_lighting_pass_depend_on_deferred_lighting_pass.dstSubpass = _main_camera_subpass_forward_lighting;
        forward_lighting_pass_depend_on_deferred_lighting_pass.srcStageMask =
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        forward_lighting_pass_depend_on_deferred_lighting_pass.dstStageMask =
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        forward_lighting_pass_depend_on_deferred_lighting_pass.srcAccessMask =
            VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        forward_lighting_pass_depend_on_deferred_lighting_pass.dstAccessMask =
            VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
        forward_lighting_pass_depend_on_deferred_lighting_pass.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        VkSubpassDependency& tone_mapping_pass_depend_on_lighting_pass = dependencies[3];
        tone_mapping_pass_depend_on_lighting_pass.srcSubpass = _main_camera_subpass_forward_lighting;
        tone_mapping_pass_depend_on_lighting_pass.dstSubpass = _main_camera_subpass_tone_mapping;
        tone_mapping_pass_depend_on_lighting_pass.srcStageMask =
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        tone_mapping_pass_depend_on_lighting_pass.dstStageMask =
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        tone_mapping_pass_depend_on_lighting_pass.srcAccessMask =
            VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        tone_mapping_pass_depend_on_lighting_pass.dstAccessMask =
            VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
        tone_mapping_pass_depend_on_lighting_pass.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        VkSubpassDependency& color_grading_pass_depend_on_tone_mapping_pass = dependencies[4];
        color_grading_pass_depend_on_tone_mapping_pass.srcSubpass = _main_camera_subpass_tone_mapping;
        color_grading_pass_depend_on_tone_mapping_pass.dstSubpass = _main_camera_subpass_color_grading;
        color_grading_pass_depend_on_tone_mapping_pass.srcStageMask =
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        color_grading_pass_depend_on_tone_mapping_pass.dstStageMask =
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        color_grading_pass_depend_on_tone_mapping_pass.srcAccessMask =
            VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        color_grading_pass_depend_on_tone_mapping_pass.dstAccessMask =
            VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
        color_grading_pass_depend_on_tone_mapping_pass.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        VkSubpassDependency& fxaa_pass_depend_on_color_grading_pass = dependencies[5];
        fxaa_pass_depend_on_color_grading_pass.srcSubpass = _main_camera_subpass_color_grading;
        fxaa_pass_depend_on_color_grading_pass.dstSubpass = _main_camera_subpass_fxaa;
        fxaa_pass_depend_on_color_grading_pass.srcStageMask =
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        fxaa_pass_depend_on_color_grading_pass.dstStageMask =
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        fxaa_pass_depend_on_color_grading_pass.srcAccessMask =
            VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        fxaa_pass_depend_on_color_grading_pass.dstAccessMask =
            VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;

        VkSubpassDependency& ui_pass_depend_on_fxaa_pass = dependencies[6];
        ui_pass_depend_on_fxaa_pass.srcSubpass = _main_camera_subpass_fxaa;
        ui_pass_depend_on_fxaa_pass.dstSubpass = _main_camera_subpass_ui;
        ui_pass_depend_on_fxaa_pass.srcStageMask =
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        ui_pass_depend_on_fxaa_pass.dstStageMask =
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        ui_pass_depend_on_fxaa_pass.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        ui_pass_depend_on_fxaa_pass.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
        ui_pass_depend_on_fxaa_pass.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        VkSubpassDependency& combine_ui_pass_depend_on_ui_pass = dependencies[7];
        combine_ui_pass_depend_on_ui_pass.srcSubpass = _main_camera_subpass_ui;
        combine_ui_pass_depend_on_ui_pass.dstSubpass = _main_camera_subpass_combine_ui;
        combine_ui_pass_depend_on_ui_pass.srcStageMask =
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        combine_ui_pass_depend_on_ui_pass.dstStageMask =
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        combine_ui_pass_depend_on_ui_pass.srcAccessMask =
            VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        combine_ui_pass_depend_on_ui_pass.dstAccessMask =
            VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
        combine_ui_pass_depend_on_ui_pass.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        VkRenderPassCreateInfo renderpass_create_info{};
        renderpass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderpass_create_info.attachmentCount = (sizeof(Attachments) / sizeof(Attachments[0]));
        renderpass_create_info.pAttachments = Attachments;
        renderpass_create_info.subpassCount = (sizeof(subpasses) / sizeof(subpasses[0]));
        renderpass_create_info.pSubpasses = subpasses;
        renderpass_create_info.dependencyCount = (sizeof(dependencies) / sizeof(dependencies[0]));
        renderpass_create_info.pDependencies = dependencies;

        if (vkCreateRenderPass(m_VulkanRhi->m_Device, &renderpass_create_info, nullptr, &m_Framebuffer.RenderPass) !=
            VK_SUCCESS)
        {
            throw std::runtime_error("failed to create render pass");
        }
	}
	void MainCameraPass::SetupDescriptorSetLayout()
	{
        m_DescriptorInfos.resize(_layout_type_count);
        
        {
            VkDescriptorSetLayoutBinding mesh_mesh_layout_bindings[1];

            VkDescriptorSetLayoutBinding& mesh_mesh_layout_uniform_buffer_binding = mesh_mesh_layout_bindings[0];
            mesh_mesh_layout_uniform_buffer_binding.binding = 0;
            mesh_mesh_layout_uniform_buffer_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            mesh_mesh_layout_uniform_buffer_binding.descriptorCount = 1;
            mesh_mesh_layout_uniform_buffer_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
            mesh_mesh_layout_uniform_buffer_binding.pImmutableSamplers = NULL;

            VkDescriptorSetLayoutCreateInfo mesh_mesh_layout_create_info{};
            mesh_mesh_layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            mesh_mesh_layout_create_info.bindingCount = 1;
            mesh_mesh_layout_create_info.pBindings = mesh_mesh_layout_bindings;

            if (vkCreateDescriptorSetLayout(m_VulkanRhi->m_Device,
                &mesh_mesh_layout_create_info,
                NULL,
                &m_DescriptorInfos[_per_mesh].Layout) != VK_SUCCESS)
            {
                throw std::runtime_error("create mesh mesh layout");
            }
        }

        {
            VkDescriptorSetLayoutBinding mesh_global_layout_bindings[8];

            VkDescriptorSetLayoutBinding& mesh_global_layout_perframe_storage_buffer_binding =
                mesh_global_layout_bindings[0];
            mesh_global_layout_perframe_storage_buffer_binding.binding = 0;
            mesh_global_layout_perframe_storage_buffer_binding.descriptorType =
                VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
            mesh_global_layout_perframe_storage_buffer_binding.descriptorCount = 1;
            mesh_global_layout_perframe_storage_buffer_binding.stageFlags =
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
            mesh_global_layout_perframe_storage_buffer_binding.pImmutableSamplers = NULL;

            VkDescriptorSetLayoutBinding& mesh_global_layout_perdrawcall_storage_buffer_binding =
                mesh_global_layout_bindings[1];
            mesh_global_layout_perdrawcall_storage_buffer_binding.binding = 1;
            mesh_global_layout_perdrawcall_storage_buffer_binding.descriptorType =
                VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
            mesh_global_layout_perdrawcall_storage_buffer_binding.descriptorCount = 1;
            mesh_global_layout_perdrawcall_storage_buffer_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
            mesh_global_layout_perdrawcall_storage_buffer_binding.pImmutableSamplers = NULL;

            VkDescriptorSetLayoutBinding& mesh_global_layout_per_drawcall_vertex_blending_storage_buffer_binding =
                mesh_global_layout_bindings[2];
            mesh_global_layout_per_drawcall_vertex_blending_storage_buffer_binding.binding = 2;
            mesh_global_layout_per_drawcall_vertex_blending_storage_buffer_binding.descriptorType =
                VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
            mesh_global_layout_per_drawcall_vertex_blending_storage_buffer_binding.descriptorCount = 1;
            mesh_global_layout_per_drawcall_vertex_blending_storage_buffer_binding.stageFlags =
                VK_SHADER_STAGE_VERTEX_BIT;
            mesh_global_layout_per_drawcall_vertex_blending_storage_buffer_binding.pImmutableSamplers = NULL;

            VkDescriptorSetLayoutBinding& mesh_global_layout_brdfLUT_texture_binding = mesh_global_layout_bindings[3];
            mesh_global_layout_brdfLUT_texture_binding.binding = 3;
            mesh_global_layout_brdfLUT_texture_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            mesh_global_layout_brdfLUT_texture_binding.descriptorCount = 1;
            mesh_global_layout_brdfLUT_texture_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
            mesh_global_layout_brdfLUT_texture_binding.pImmutableSamplers = NULL;

            VkDescriptorSetLayoutBinding& mesh_global_layout_irradiance_texture_binding =
                mesh_global_layout_bindings[4];
            mesh_global_layout_irradiance_texture_binding = mesh_global_layout_brdfLUT_texture_binding;
            mesh_global_layout_irradiance_texture_binding.binding = 4;

            VkDescriptorSetLayoutBinding& mesh_global_layout_specular_texture_binding = mesh_global_layout_bindings[5];
            mesh_global_layout_specular_texture_binding = mesh_global_layout_brdfLUT_texture_binding;
            mesh_global_layout_specular_texture_binding.binding = 5;

            VkDescriptorSetLayoutBinding& mesh_global_layout_point_light_shadow_texture_binding =
                mesh_global_layout_bindings[6];
            mesh_global_layout_point_light_shadow_texture_binding = mesh_global_layout_brdfLUT_texture_binding;
            mesh_global_layout_point_light_shadow_texture_binding.binding = 6;

            VkDescriptorSetLayoutBinding& mesh_global_layout_directional_light_shadow_texture_binding =
                mesh_global_layout_bindings[7];
            mesh_global_layout_directional_light_shadow_texture_binding = mesh_global_layout_brdfLUT_texture_binding;
            mesh_global_layout_directional_light_shadow_texture_binding.binding = 7;

            VkDescriptorSetLayoutCreateInfo mesh_global_layout_create_info;
            mesh_global_layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            mesh_global_layout_create_info.pNext = NULL;
            mesh_global_layout_create_info.flags = 0;
            mesh_global_layout_create_info.bindingCount =
                (sizeof(mesh_global_layout_bindings) / sizeof(mesh_global_layout_bindings[0]));
            mesh_global_layout_create_info.pBindings = mesh_global_layout_bindings;

            if (VK_SUCCESS != vkCreateDescriptorSetLayout(m_VulkanRhi->m_Device,
                &mesh_global_layout_create_info,
                NULL,
                &m_DescriptorInfos[_mesh_global].Layout))
            {
                throw std::runtime_error("create mesh global layout");
            }
        }

        {
            VkDescriptorSetLayoutBinding mesh_material_layout_bindings[6];

            // (set = 2, binding = 0 in fragment shader)
            VkDescriptorSetLayoutBinding& mesh_material_layout_uniform_buffer_binding =
                mesh_material_layout_bindings[0];
            mesh_material_layout_uniform_buffer_binding.binding = 0;
            mesh_material_layout_uniform_buffer_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            mesh_material_layout_uniform_buffer_binding.descriptorCount = 1;
            mesh_material_layout_uniform_buffer_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
            mesh_material_layout_uniform_buffer_binding.pImmutableSamplers = nullptr;

            // (set = 2, binding = 1 in fragment shader)
            VkDescriptorSetLayoutBinding& mesh_material_layout_base_color_texture_binding =
                mesh_material_layout_bindings[1];
            mesh_material_layout_base_color_texture_binding.binding = 1;
            mesh_material_layout_base_color_texture_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            mesh_material_layout_base_color_texture_binding.descriptorCount = 1;
            mesh_material_layout_base_color_texture_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
            mesh_material_layout_base_color_texture_binding.pImmutableSamplers = nullptr;

            // (set = 2, binding = 2 in fragment shader)
            VkDescriptorSetLayoutBinding& mesh_material_layout_metallic_roughness_texture_binding =
                mesh_material_layout_bindings[2];
            mesh_material_layout_metallic_roughness_texture_binding = mesh_material_layout_base_color_texture_binding;
            mesh_material_layout_metallic_roughness_texture_binding.binding = 2;

            // (set = 2, binding = 3 in fragment shader)
            VkDescriptorSetLayoutBinding& mesh_material_layout_normal_roughness_texture_binding =
                mesh_material_layout_bindings[3];
            mesh_material_layout_normal_roughness_texture_binding = mesh_material_layout_base_color_texture_binding;
            mesh_material_layout_normal_roughness_texture_binding.binding = 3;

            // (set = 2, binding = 4 in fragment shader)
            VkDescriptorSetLayoutBinding& mesh_material_layout_occlusion_texture_binding =
                mesh_material_layout_bindings[4];
            mesh_material_layout_occlusion_texture_binding = mesh_material_layout_base_color_texture_binding;
            mesh_material_layout_occlusion_texture_binding.binding = 4;

            // (set = 2, binding = 5 in fragment shader)
            VkDescriptorSetLayoutBinding& mesh_material_layout_emissive_texture_binding =
                mesh_material_layout_bindings[5];
            mesh_material_layout_emissive_texture_binding = mesh_material_layout_base_color_texture_binding;
            mesh_material_layout_emissive_texture_binding.binding = 5;

            VkDescriptorSetLayoutCreateInfo mesh_material_layout_create_info;
            mesh_material_layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            mesh_material_layout_create_info.pNext = NULL;
            mesh_material_layout_create_info.flags = 0;
            mesh_material_layout_create_info.bindingCount = 6;
            mesh_material_layout_create_info.pBindings = mesh_material_layout_bindings;

            if (vkCreateDescriptorSetLayout(m_VulkanRhi->m_Device,
                &mesh_material_layout_create_info,
                nullptr,
                &m_DescriptorInfos[_mesh_per_material].Layout) != VK_SUCCESS)

            {
                throw std::runtime_error("create mesh material layout");
            }
        }

        {
            VkDescriptorSetLayoutBinding skybox_layout_bindings[2];

            VkDescriptorSetLayoutBinding& skybox_layout_perframe_storage_buffer_binding = skybox_layout_bindings[0];
            skybox_layout_perframe_storage_buffer_binding.binding = 0;
            skybox_layout_perframe_storage_buffer_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
            skybox_layout_perframe_storage_buffer_binding.descriptorCount = 1;
            skybox_layout_perframe_storage_buffer_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
            skybox_layout_perframe_storage_buffer_binding.pImmutableSamplers = NULL;

            VkDescriptorSetLayoutBinding& skybox_layout_specular_texture_binding = skybox_layout_bindings[1];
            skybox_layout_specular_texture_binding.binding = 1;
            skybox_layout_specular_texture_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            skybox_layout_specular_texture_binding.descriptorCount = 1;
            skybox_layout_specular_texture_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
            skybox_layout_specular_texture_binding.pImmutableSamplers = NULL;

            VkDescriptorSetLayoutCreateInfo skybox_layout_create_info{};
            skybox_layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            skybox_layout_create_info.bindingCount = 2;
            skybox_layout_create_info.pBindings = skybox_layout_bindings;

            if (VK_SUCCESS !=
                vkCreateDescriptorSetLayout(
                    m_VulkanRhi->m_Device, &skybox_layout_create_info, NULL, &m_DescriptorInfos[_skybox].Layout))
            {
                throw std::runtime_error("create skybox layout");
            }
        }

        {
            VkDescriptorSetLayoutBinding axis_layout_bindings[2];

            VkDescriptorSetLayoutBinding& axis_layout_perframe_storage_buffer_binding = axis_layout_bindings[0];
            axis_layout_perframe_storage_buffer_binding.binding = 0;
            axis_layout_perframe_storage_buffer_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
            axis_layout_perframe_storage_buffer_binding.descriptorCount = 1;
            axis_layout_perframe_storage_buffer_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
            axis_layout_perframe_storage_buffer_binding.pImmutableSamplers = NULL;

            VkDescriptorSetLayoutBinding& axis_layout_storage_buffer_binding = axis_layout_bindings[1];
            axis_layout_storage_buffer_binding.binding = 1;
            axis_layout_storage_buffer_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            axis_layout_storage_buffer_binding.descriptorCount = 1;
            axis_layout_storage_buffer_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
            axis_layout_storage_buffer_binding.pImmutableSamplers = NULL;

            VkDescriptorSetLayoutCreateInfo axis_layout_create_info{};
            axis_layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            axis_layout_create_info.bindingCount = 2;
            axis_layout_create_info.pBindings = axis_layout_bindings;

            if (VK_SUCCESS !=
                vkCreateDescriptorSetLayout(
                    m_VulkanRhi->m_Device, &axis_layout_create_info, NULL, &m_DescriptorInfos[_axis].Layout))
            {
                throw std::runtime_error("create axis layout");
            }
        }

        {
            VkDescriptorSetLayoutBinding gbuffer_lighting_global_layout_bindings[4];

            VkDescriptorSetLayoutBinding& gbuffer_normal_global_layout_input_attachment_binding =
                gbuffer_lighting_global_layout_bindings[0];
            gbuffer_normal_global_layout_input_attachment_binding.binding = 0;
            gbuffer_normal_global_layout_input_attachment_binding.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
            gbuffer_normal_global_layout_input_attachment_binding.descriptorCount = 1;
            gbuffer_normal_global_layout_input_attachment_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

            VkDescriptorSetLayoutBinding&
                gbuffer_metallic_roughness_shadingmodeid_global_layout_input_attachment_binding =
                gbuffer_lighting_global_layout_bindings[1];
            gbuffer_metallic_roughness_shadingmodeid_global_layout_input_attachment_binding.binding = 1;
            gbuffer_metallic_roughness_shadingmodeid_global_layout_input_attachment_binding.descriptorType =
                VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
            gbuffer_metallic_roughness_shadingmodeid_global_layout_input_attachment_binding.descriptorCount = 1;
            gbuffer_metallic_roughness_shadingmodeid_global_layout_input_attachment_binding.stageFlags =
                VK_SHADER_STAGE_FRAGMENT_BIT;

            VkDescriptorSetLayoutBinding& gbuffer_albedo_global_layout_input_attachment_binding =
                gbuffer_lighting_global_layout_bindings[2];
            gbuffer_albedo_global_layout_input_attachment_binding.binding = 2;
            gbuffer_albedo_global_layout_input_attachment_binding.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
            gbuffer_albedo_global_layout_input_attachment_binding.descriptorCount = 1;
            gbuffer_albedo_global_layout_input_attachment_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

            VkDescriptorSetLayoutBinding& gbuffer_depth_global_layout_input_attachment_binding =
                gbuffer_lighting_global_layout_bindings[3];
            gbuffer_depth_global_layout_input_attachment_binding.binding = 3;
            gbuffer_depth_global_layout_input_attachment_binding.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
            gbuffer_depth_global_layout_input_attachment_binding.descriptorCount = 1;
            gbuffer_depth_global_layout_input_attachment_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

            VkDescriptorSetLayoutCreateInfo gbuffer_lighting_global_layout_create_info;
            gbuffer_lighting_global_layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            gbuffer_lighting_global_layout_create_info.pNext = NULL;
            gbuffer_lighting_global_layout_create_info.flags = 0;
            gbuffer_lighting_global_layout_create_info.bindingCount =
                sizeof(gbuffer_lighting_global_layout_bindings) / sizeof(gbuffer_lighting_global_layout_bindings[0]);
            gbuffer_lighting_global_layout_create_info.pBindings = gbuffer_lighting_global_layout_bindings;

            if (VK_SUCCESS != vkCreateDescriptorSetLayout(m_VulkanRhi->m_Device,
                &gbuffer_lighting_global_layout_create_info,
                NULL,
                &m_DescriptorInfos[_deferred_lighting].Layout))
            {
                throw std::runtime_error("create deferred lighting global layout");
            }
        }
	}
	void MainCameraPass::SetupPipelines()
	{
        m_RenderPipelines.resize(_render_pipeline_type_count);

        // mesh gbuffer
        {
            VkDescriptorSetLayout      descriptorset_layouts[3] = { m_DescriptorInfos[_mesh_global].Layout,
                                                              m_DescriptorInfos[_per_mesh].Layout,
                                                              m_DescriptorInfos[_mesh_per_material].Layout };
            VkPipelineLayoutCreateInfo pipeline_layout_create_info{};
            pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            pipeline_layout_create_info.setLayoutCount = 3;
            pipeline_layout_create_info.pSetLayouts = descriptorset_layouts;

            if (vkCreatePipelineLayout(m_VulkanRhi->m_Device,
                &pipeline_layout_create_info,
                nullptr,
                &m_RenderPipelines[_render_pipeline_type_mesh_gbuffer].Layout) != VK_SUCCESS)
            {
                throw std::runtime_error("create mesh gbuffer pipeline layout");
            }

            VkShaderModule vert_shader_module = VulkanUtil::CreateShaderModule(m_VulkanRhi->m_Device, MESH_VERT);
            VkShaderModule frag_shader_module =
                VulkanUtil::CreateShaderModule(m_VulkanRhi->m_Device, MESH_GBUFFER_FRAG);

            VkPipelineShaderStageCreateInfo vert_pipeline_shader_stage_create_info{};
            vert_pipeline_shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            vert_pipeline_shader_stage_create_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
            vert_pipeline_shader_stage_create_info.module = vert_shader_module;
            vert_pipeline_shader_stage_create_info.pName = "main";

            VkPipelineShaderStageCreateInfo frag_pipeline_shader_stage_create_info{};
            frag_pipeline_shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            frag_pipeline_shader_stage_create_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            frag_pipeline_shader_stage_create_info.module = frag_shader_module;
            frag_pipeline_shader_stage_create_info.pName = "main";

            VkPipelineShaderStageCreateInfo shader_stages[] = { vert_pipeline_shader_stage_create_info,
                                                               frag_pipeline_shader_stage_create_info };

            auto                                 vertex_binding_descriptions = MeshVertex::getBindingDescriptions();
            auto                                 vertex_attribute_descriptions = MeshVertex::getAttributeDescriptions();
            VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info{};
            vertex_input_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            vertex_input_state_create_info.vertexBindingDescriptionCount = vertex_binding_descriptions.size();
            vertex_input_state_create_info.pVertexBindingDescriptions = &vertex_binding_descriptions[0];
            vertex_input_state_create_info.vertexAttributeDescriptionCount = vertex_attribute_descriptions.size();
            vertex_input_state_create_info.pVertexAttributeDescriptions = &vertex_attribute_descriptions[0];

            VkPipelineInputAssemblyStateCreateInfo input_assembly_create_info{};
            input_assembly_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
            input_assembly_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            input_assembly_create_info.primitiveRestartEnable = VK_FALSE;

            VkPipelineViewportStateCreateInfo viewport_state_create_info{};
            viewport_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
            viewport_state_create_info.viewportCount = 1;
            viewport_state_create_info.pViewports = &m_VulkanRhi->m_Viewport;
            viewport_state_create_info.scissorCount = 1;
            viewport_state_create_info.pScissors = &m_VulkanRhi->m_Scissor;

            VkPipelineRasterizationStateCreateInfo rasterization_state_create_info{};
            rasterization_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
            rasterization_state_create_info.depthClampEnable = VK_FALSE;
            rasterization_state_create_info.rasterizerDiscardEnable = VK_FALSE;
            rasterization_state_create_info.polygonMode = VK_POLYGON_MODE_FILL;
            rasterization_state_create_info.lineWidth = 1.0f;
            rasterization_state_create_info.cullMode = VK_CULL_MODE_BACK_BIT;
            rasterization_state_create_info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
            rasterization_state_create_info.depthBiasEnable = VK_FALSE;
            rasterization_state_create_info.depthBiasConstantFactor = 0.0f;
            rasterization_state_create_info.depthBiasClamp = 0.0f;
            rasterization_state_create_info.depthBiasSlopeFactor = 0.0f;

            VkPipelineMultisampleStateCreateInfo multisample_state_create_info{};
            multisample_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
            multisample_state_create_info.sampleShadingEnable = VK_FALSE;
            multisample_state_create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

            VkPipelineColorBlendAttachmentState color_blend_Attachments[3] = {};
            color_blend_Attachments[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
            color_blend_Attachments[0].blendEnable = VK_FALSE;
            color_blend_Attachments[0].srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
            color_blend_Attachments[0].dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
            color_blend_Attachments[0].colorBlendOp = VK_BLEND_OP_ADD;
            color_blend_Attachments[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            color_blend_Attachments[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
            color_blend_Attachments[0].alphaBlendOp = VK_BLEND_OP_ADD;
            color_blend_Attachments[1].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
            color_blend_Attachments[1].blendEnable = VK_FALSE;
            color_blend_Attachments[1].srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
            color_blend_Attachments[1].dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
            color_blend_Attachments[1].colorBlendOp = VK_BLEND_OP_ADD;
            color_blend_Attachments[1].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            color_blend_Attachments[1].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
            color_blend_Attachments[1].alphaBlendOp = VK_BLEND_OP_ADD;
            color_blend_Attachments[2].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
            color_blend_Attachments[2].blendEnable = VK_FALSE;
            color_blend_Attachments[2].srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
            color_blend_Attachments[2].dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
            color_blend_Attachments[2].colorBlendOp = VK_BLEND_OP_ADD;
            color_blend_Attachments[2].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            color_blend_Attachments[2].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
            color_blend_Attachments[2].alphaBlendOp = VK_BLEND_OP_ADD;

            VkPipelineColorBlendStateCreateInfo color_blend_state_create_info = {};
            color_blend_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
            color_blend_state_create_info.logicOpEnable = VK_FALSE;
            color_blend_state_create_info.logicOp = VK_LOGIC_OP_COPY;
            color_blend_state_create_info.attachmentCount =
                sizeof(color_blend_Attachments) / sizeof(color_blend_Attachments[0]);
            color_blend_state_create_info.pAttachments = &color_blend_Attachments[0];
            color_blend_state_create_info.blendConstants[0] = 0.0f;
            color_blend_state_create_info.blendConstants[1] = 0.0f;
            color_blend_state_create_info.blendConstants[2] = 0.0f;
            color_blend_state_create_info.blendConstants[3] = 0.0f;

            VkPipelineDepthStencilStateCreateInfo depth_stencil_create_info{};
            depth_stencil_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
            depth_stencil_create_info.depthTestEnable = VK_TRUE;
            depth_stencil_create_info.depthWriteEnable = VK_TRUE;
            depth_stencil_create_info.depthCompareOp = VK_COMPARE_OP_LESS;
            depth_stencil_create_info.depthBoundsTestEnable = VK_FALSE;
            depth_stencil_create_info.stencilTestEnable = VK_FALSE;

            VkDynamicState                   dynamic_states[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
            VkPipelineDynamicStateCreateInfo dynamic_state_create_info{};
            dynamic_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
            dynamic_state_create_info.dynamicStateCount = 2;
            dynamic_state_create_info.pDynamicStates = dynamic_states;

            VkGraphicsPipelineCreateInfo pipelineInfo{};
            pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
            pipelineInfo.stageCount = 2;
            pipelineInfo.pStages = shader_stages;
            pipelineInfo.pVertexInputState = &vertex_input_state_create_info;
            pipelineInfo.pInputAssemblyState = &input_assembly_create_info;
            pipelineInfo.pViewportState = &viewport_state_create_info;
            pipelineInfo.pRasterizationState = &rasterization_state_create_info;
            pipelineInfo.pMultisampleState = &multisample_state_create_info;
            pipelineInfo.pColorBlendState = &color_blend_state_create_info;
            pipelineInfo.pDepthStencilState = &depth_stencil_create_info;
            pipelineInfo.layout = m_RenderPipelines[_render_pipeline_type_mesh_gbuffer].Layout;
            pipelineInfo.renderPass = m_Framebuffer.RenderPass;
            pipelineInfo.subpass = _main_camera_subpass_basepass;
            pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
            pipelineInfo.pDynamicState = &dynamic_state_create_info;

            if (vkCreateGraphicsPipelines(m_VulkanRhi->m_Device,
                VK_NULL_HANDLE,
                1,
                &pipelineInfo,
                nullptr,
                &m_RenderPipelines[_render_pipeline_type_mesh_gbuffer].Pipeline) !=
                VK_SUCCESS)
            {
                throw std::runtime_error("create mesh gbuffer graphics pipeline");
            }

            vkDestroyShaderModule(m_VulkanRhi->m_Device, vert_shader_module, nullptr);
            vkDestroyShaderModule(m_VulkanRhi->m_Device, frag_shader_module, nullptr);
        }

        // deferred lighting
        {
            VkDescriptorSetLayout      descriptorset_layouts[3] = { m_DescriptorInfos[_mesh_global].Layout,
                                                              m_DescriptorInfos[_deferred_lighting].Layout,
                                                              m_DescriptorInfos[_skybox].Layout };
            VkPipelineLayoutCreateInfo pipeline_layout_create_info{};
            pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            pipeline_layout_create_info.setLayoutCount =
                sizeof(descriptorset_layouts) / sizeof(descriptorset_layouts[0]);
            pipeline_layout_create_info.pSetLayouts = descriptorset_layouts;

            if (vkCreatePipelineLayout(m_VulkanRhi->m_Device,
                &pipeline_layout_create_info,
                nullptr,
                &m_RenderPipelines[_render_pipeline_type_deferred_lighting].Layout) !=
                VK_SUCCESS)
            {
                throw std::runtime_error("create deferred lighting pipeline layout");
            }

            VkShaderModule vert_shader_module =
                VulkanUtil::CreateShaderModule(m_VulkanRhi->m_Device, DEFERRED_LIGHTING_VERT);
            VkShaderModule frag_shader_module =
                VulkanUtil::CreateShaderModule(m_VulkanRhi->m_Device, DEFERRED_LIGHTING_FRAG);

            VkPipelineShaderStageCreateInfo vert_pipeline_shader_stage_create_info{};
            vert_pipeline_shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            vert_pipeline_shader_stage_create_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
            vert_pipeline_shader_stage_create_info.module = vert_shader_module;
            vert_pipeline_shader_stage_create_info.pName = "main";
            // vert_pipeline_shader_stage_create_info.pSpecializationInfo

            VkPipelineShaderStageCreateInfo frag_pipeline_shader_stage_create_info{};
            frag_pipeline_shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            frag_pipeline_shader_stage_create_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            frag_pipeline_shader_stage_create_info.module = frag_shader_module;
            frag_pipeline_shader_stage_create_info.pName = "main";

            VkPipelineShaderStageCreateInfo shader_stages[] = { vert_pipeline_shader_stage_create_info,
                                                               frag_pipeline_shader_stage_create_info };

            auto                                 vertex_binding_descriptions = MeshVertex::getBindingDescriptions();
            auto                                 vertex_attribute_descriptions = MeshVertex::getAttributeDescriptions();
            VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info{};
            vertex_input_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            vertex_input_state_create_info.vertexBindingDescriptionCount = 0;
            vertex_input_state_create_info.pVertexBindingDescriptions = NULL;
            vertex_input_state_create_info.vertexBindingDescriptionCount = 0;
            vertex_input_state_create_info.pVertexAttributeDescriptions = NULL;

            VkPipelineInputAssemblyStateCreateInfo input_assembly_create_info{};
            input_assembly_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
            input_assembly_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            input_assembly_create_info.primitiveRestartEnable = VK_FALSE;

            VkPipelineViewportStateCreateInfo viewport_state_create_info{};
            viewport_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
            viewport_state_create_info.viewportCount = 1;
            viewport_state_create_info.pViewports = &m_VulkanRhi->m_Viewport;
            viewport_state_create_info.scissorCount = 1;
            viewport_state_create_info.pScissors = &m_VulkanRhi->m_Scissor;

            VkPipelineRasterizationStateCreateInfo rasterization_state_create_info{};
            rasterization_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
            rasterization_state_create_info.depthClampEnable = VK_FALSE;
            rasterization_state_create_info.rasterizerDiscardEnable = VK_FALSE;
            rasterization_state_create_info.polygonMode = VK_POLYGON_MODE_FILL;
            rasterization_state_create_info.lineWidth = 1.0f;
            rasterization_state_create_info.cullMode = VK_CULL_MODE_BACK_BIT;
            rasterization_state_create_info.frontFace = VK_FRONT_FACE_CLOCKWISE;
            rasterization_state_create_info.depthBiasEnable = VK_FALSE;
            rasterization_state_create_info.depthBiasConstantFactor = 0.0f;
            rasterization_state_create_info.depthBiasClamp = 0.0f;
            rasterization_state_create_info.depthBiasSlopeFactor = 0.0f;

            VkPipelineMultisampleStateCreateInfo multisample_state_create_info{};
            multisample_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
            multisample_state_create_info.sampleShadingEnable = VK_FALSE;
            multisample_state_create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

            VkPipelineColorBlendAttachmentState color_blend_Attachments[1] = {};
            color_blend_Attachments[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
            color_blend_Attachments[0].blendEnable = VK_FALSE;
            color_blend_Attachments[0].srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
            color_blend_Attachments[0].dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
            color_blend_Attachments[0].colorBlendOp = VK_BLEND_OP_ADD;
            color_blend_Attachments[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            color_blend_Attachments[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            color_blend_Attachments[0].alphaBlendOp = VK_BLEND_OP_ADD;

            VkPipelineColorBlendStateCreateInfo color_blend_state_create_info = {};
            color_blend_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
            color_blend_state_create_info.logicOpEnable = VK_FALSE;
            color_blend_state_create_info.logicOp = VK_LOGIC_OP_COPY;
            color_blend_state_create_info.attachmentCount =
                sizeof(color_blend_Attachments) / sizeof(color_blend_Attachments[0]);
            color_blend_state_create_info.pAttachments = &color_blend_Attachments[0];
            color_blend_state_create_info.blendConstants[0] = 0.0f;
            color_blend_state_create_info.blendConstants[1] = 0.0f;
            color_blend_state_create_info.blendConstants[2] = 0.0f;
            color_blend_state_create_info.blendConstants[3] = 0.0f;

            VkPipelineDepthStencilStateCreateInfo depth_stencil_create_info{};
            depth_stencil_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
            depth_stencil_create_info.depthTestEnable = VK_FALSE;
            depth_stencil_create_info.depthWriteEnable = VK_FALSE;
            depth_stencil_create_info.depthCompareOp = VK_COMPARE_OP_ALWAYS;
            depth_stencil_create_info.depthBoundsTestEnable = VK_FALSE;
            depth_stencil_create_info.stencilTestEnable = VK_FALSE;

            VkDynamicState                   dynamic_states[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
            VkPipelineDynamicStateCreateInfo dynamic_state_create_info{};
            dynamic_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
            dynamic_state_create_info.dynamicStateCount = 2;
            dynamic_state_create_info.pDynamicStates = dynamic_states;

            VkGraphicsPipelineCreateInfo pipelineInfo{};
            pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
            pipelineInfo.stageCount = 2;
            pipelineInfo.pStages = shader_stages;
            pipelineInfo.pVertexInputState = &vertex_input_state_create_info;
            pipelineInfo.pInputAssemblyState = &input_assembly_create_info;
            pipelineInfo.pViewportState = &viewport_state_create_info;
            pipelineInfo.pRasterizationState = &rasterization_state_create_info;
            pipelineInfo.pMultisampleState = &multisample_state_create_info;
            pipelineInfo.pColorBlendState = &color_blend_state_create_info;
            pipelineInfo.pDepthStencilState = &depth_stencil_create_info;
            pipelineInfo.layout = m_RenderPipelines[_render_pipeline_type_deferred_lighting].Layout;
            pipelineInfo.renderPass = m_Framebuffer.RenderPass;
            pipelineInfo.subpass = _main_camera_subpass_deferred_lighting;
            pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
            pipelineInfo.pDynamicState = &dynamic_state_create_info;

            if (vkCreateGraphicsPipelines(m_VulkanRhi->m_Device,
                VK_NULL_HANDLE,
                1,
                &pipelineInfo,
                nullptr,
                &m_RenderPipelines[_render_pipeline_type_deferred_lighting].Pipeline) !=
                VK_SUCCESS)
            {
                throw std::runtime_error("create deferred lighting graphics pipeline");
            }

            vkDestroyShaderModule(m_VulkanRhi->m_Device, vert_shader_module, nullptr);
            vkDestroyShaderModule(m_VulkanRhi->m_Device, frag_shader_module, nullptr);
        }

        // mesh lighting
        {
            VkDescriptorSetLayout      descriptorset_layouts[3] = { m_DescriptorInfos[_mesh_global].Layout,
                                                              m_DescriptorInfos[_per_mesh].Layout,
                                                              m_DescriptorInfos[_mesh_per_material].Layout };
            VkPipelineLayoutCreateInfo pipeline_layout_create_info{};
            pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            pipeline_layout_create_info.setLayoutCount = 3;
            pipeline_layout_create_info.pSetLayouts = descriptorset_layouts;

            if (vkCreatePipelineLayout(m_VulkanRhi->m_Device,
                &pipeline_layout_create_info,
                nullptr,
                &m_RenderPipelines[_render_pipeline_type_mesh_lighting].Layout) != VK_SUCCESS)
            {
                throw std::runtime_error("create mesh lighting pipeline layout");
            }

            VkShaderModule vert_shader_module = VulkanUtil::CreateShaderModule(m_VulkanRhi->m_Device, MESH_VERT);
            VkShaderModule frag_shader_module = VulkanUtil::CreateShaderModule(m_VulkanRhi->m_Device, MESH_FRAG);

            VkPipelineShaderStageCreateInfo vert_pipeline_shader_stage_create_info{};
            vert_pipeline_shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            vert_pipeline_shader_stage_create_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
            vert_pipeline_shader_stage_create_info.module = vert_shader_module;
            vert_pipeline_shader_stage_create_info.pName = "main";

            VkPipelineShaderStageCreateInfo frag_pipeline_shader_stage_create_info{};
            frag_pipeline_shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            frag_pipeline_shader_stage_create_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            frag_pipeline_shader_stage_create_info.module = frag_shader_module;
            frag_pipeline_shader_stage_create_info.pName = "main";

            VkPipelineShaderStageCreateInfo shader_stages[] = { vert_pipeline_shader_stage_create_info,
                                                               frag_pipeline_shader_stage_create_info };

            auto                                 vertex_binding_descriptions = MeshVertex::getBindingDescriptions();
            auto                                 vertex_attribute_descriptions = MeshVertex::getAttributeDescriptions();
            VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info{};
            vertex_input_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            vertex_input_state_create_info.vertexBindingDescriptionCount = vertex_binding_descriptions.size();
            vertex_input_state_create_info.pVertexBindingDescriptions = &vertex_binding_descriptions[0];
            vertex_input_state_create_info.vertexAttributeDescriptionCount = vertex_attribute_descriptions.size();
            vertex_input_state_create_info.pVertexAttributeDescriptions = &vertex_attribute_descriptions[0];

            VkPipelineInputAssemblyStateCreateInfo input_assembly_create_info{};
            input_assembly_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
            input_assembly_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            input_assembly_create_info.primitiveRestartEnable = VK_FALSE;

            VkPipelineViewportStateCreateInfo viewport_state_create_info{};
            viewport_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
            viewport_state_create_info.viewportCount = 1;
            viewport_state_create_info.pViewports = &m_VulkanRhi->m_Viewport;
            viewport_state_create_info.scissorCount = 1;
            viewport_state_create_info.pScissors = &m_VulkanRhi->m_Scissor;

            VkPipelineRasterizationStateCreateInfo rasterization_state_create_info{};
            rasterization_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
            rasterization_state_create_info.depthClampEnable = VK_FALSE;
            rasterization_state_create_info.rasterizerDiscardEnable = VK_FALSE;
            rasterization_state_create_info.polygonMode = VK_POLYGON_MODE_FILL;
            rasterization_state_create_info.lineWidth = 1.0f;
            rasterization_state_create_info.cullMode = VK_CULL_MODE_BACK_BIT;
            rasterization_state_create_info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
            rasterization_state_create_info.depthBiasEnable = VK_FALSE;
            rasterization_state_create_info.depthBiasConstantFactor = 0.0f;
            rasterization_state_create_info.depthBiasClamp = 0.0f;
            rasterization_state_create_info.depthBiasSlopeFactor = 0.0f;

            VkPipelineMultisampleStateCreateInfo multisample_state_create_info{};
            multisample_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
            multisample_state_create_info.sampleShadingEnable = VK_FALSE;
            multisample_state_create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

            VkPipelineColorBlendAttachmentState color_blend_Attachments[1] = {};
            color_blend_Attachments[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
            color_blend_Attachments[0].blendEnable = VK_FALSE;
            color_blend_Attachments[0].srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
            color_blend_Attachments[0].dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
            color_blend_Attachments[0].colorBlendOp = VK_BLEND_OP_ADD;
            color_blend_Attachments[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            color_blend_Attachments[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            color_blend_Attachments[0].alphaBlendOp = VK_BLEND_OP_ADD;

            VkPipelineColorBlendStateCreateInfo color_blend_state_create_info = {};
            color_blend_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
            color_blend_state_create_info.logicOpEnable = VK_FALSE;
            color_blend_state_create_info.logicOp = VK_LOGIC_OP_COPY;
            color_blend_state_create_info.attachmentCount =
                sizeof(color_blend_Attachments) / sizeof(color_blend_Attachments[0]);
            color_blend_state_create_info.pAttachments = &color_blend_Attachments[0];
            color_blend_state_create_info.blendConstants[0] = 0.0f;
            color_blend_state_create_info.blendConstants[1] = 0.0f;
            color_blend_state_create_info.blendConstants[2] = 0.0f;
            color_blend_state_create_info.blendConstants[3] = 0.0f;

            VkPipelineDepthStencilStateCreateInfo depth_stencil_create_info{};
            depth_stencil_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
            depth_stencil_create_info.depthTestEnable = VK_TRUE;
            depth_stencil_create_info.depthWriteEnable = VK_TRUE;
            depth_stencil_create_info.depthCompareOp = VK_COMPARE_OP_LESS;
            depth_stencil_create_info.depthBoundsTestEnable = VK_FALSE;
            depth_stencil_create_info.stencilTestEnable = VK_FALSE;

            VkDynamicState                   dynamic_states[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
            VkPipelineDynamicStateCreateInfo dynamic_state_create_info{};
            dynamic_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
            dynamic_state_create_info.dynamicStateCount = 2;
            dynamic_state_create_info.pDynamicStates = dynamic_states;

            VkGraphicsPipelineCreateInfo pipelineInfo{};
            pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
            pipelineInfo.stageCount = 2;
            pipelineInfo.pStages = shader_stages;
            pipelineInfo.pVertexInputState = &vertex_input_state_create_info;
            pipelineInfo.pInputAssemblyState = &input_assembly_create_info;
            pipelineInfo.pViewportState = &viewport_state_create_info;
            pipelineInfo.pRasterizationState = &rasterization_state_create_info;
            pipelineInfo.pMultisampleState = &multisample_state_create_info;
            pipelineInfo.pColorBlendState = &color_blend_state_create_info;
            pipelineInfo.pDepthStencilState = &depth_stencil_create_info;
            pipelineInfo.layout = m_RenderPipelines[_render_pipeline_type_mesh_lighting].Layout;
            pipelineInfo.renderPass = m_Framebuffer.RenderPass;
            pipelineInfo.subpass = _main_camera_subpass_forward_lighting;
            pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
            pipelineInfo.pDynamicState = &dynamic_state_create_info;

            if (vkCreateGraphicsPipelines(m_VulkanRhi->m_Device,
                VK_NULL_HANDLE,
                1,
                &pipelineInfo,
                nullptr,
                &m_RenderPipelines[_render_pipeline_type_mesh_lighting].Pipeline) !=
                VK_SUCCESS)
            {
                throw std::runtime_error("create mesh lighting graphics pipeline");
            }

            vkDestroyShaderModule(m_VulkanRhi->m_Device, vert_shader_module, nullptr);
            vkDestroyShaderModule(m_VulkanRhi->m_Device, frag_shader_module, nullptr);
        }

        // skybox
        {
            VkDescriptorSetLayout      descriptorset_layouts[1] = { m_DescriptorInfos[_skybox].Layout };
            VkPipelineLayoutCreateInfo pipeline_layout_create_info{};
            pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            pipeline_layout_create_info.setLayoutCount = 1;
            pipeline_layout_create_info.pSetLayouts = descriptorset_layouts;

            if (vkCreatePipelineLayout(m_VulkanRhi->m_Device,
                &pipeline_layout_create_info,
                nullptr,
                &m_RenderPipelines[_render_pipeline_type_skybox].Layout) != VK_SUCCESS)
            {
                throw std::runtime_error("create skybox pipeline layout");
            }

            VkShaderModule vert_shader_module = VulkanUtil::CreateShaderModule(m_VulkanRhi->m_Device, SKYBOX_VERT);
            VkShaderModule frag_shader_module = VulkanUtil::CreateShaderModule(m_VulkanRhi->m_Device, SKYBOX_FRAG);

            VkPipelineShaderStageCreateInfo vert_pipeline_shader_stage_create_info{};
            vert_pipeline_shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            vert_pipeline_shader_stage_create_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
            vert_pipeline_shader_stage_create_info.module = vert_shader_module;
            vert_pipeline_shader_stage_create_info.pName = "main";
            // vert_pipeline_shader_stage_create_info.pSpecializationInfo

            VkPipelineShaderStageCreateInfo frag_pipeline_shader_stage_create_info{};
            frag_pipeline_shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            frag_pipeline_shader_stage_create_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            frag_pipeline_shader_stage_create_info.module = frag_shader_module;
            frag_pipeline_shader_stage_create_info.pName = "main";

            VkPipelineShaderStageCreateInfo shader_stages[] = { vert_pipeline_shader_stage_create_info,
                                                               frag_pipeline_shader_stage_create_info };

            auto                                 vertex_binding_descriptions = MeshVertex::getBindingDescriptions();
            auto                                 vertex_attribute_descriptions = MeshVertex::getAttributeDescriptions();
            VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info{};
            vertex_input_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            vertex_input_state_create_info.vertexBindingDescriptionCount = 0;
            vertex_input_state_create_info.pVertexBindingDescriptions = NULL;
            vertex_input_state_create_info.vertexAttributeDescriptionCount = 0;
            vertex_input_state_create_info.pVertexAttributeDescriptions = NULL;

            VkPipelineInputAssemblyStateCreateInfo input_assembly_create_info{};
            input_assembly_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
            input_assembly_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            input_assembly_create_info.primitiveRestartEnable = VK_FALSE;

            VkPipelineViewportStateCreateInfo viewport_state_create_info{};
            viewport_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
            viewport_state_create_info.viewportCount = 1;
            viewport_state_create_info.pViewports = &m_VulkanRhi->m_Viewport;
            viewport_state_create_info.scissorCount = 1;
            viewport_state_create_info.pScissors = &m_VulkanRhi->m_Scissor;

            VkPipelineRasterizationStateCreateInfo rasterization_state_create_info{};
            rasterization_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
            rasterization_state_create_info.depthClampEnable = VK_FALSE;
            rasterization_state_create_info.rasterizerDiscardEnable = VK_FALSE;
            rasterization_state_create_info.polygonMode = VK_POLYGON_MODE_FILL;
            rasterization_state_create_info.lineWidth = 1.0f;
            rasterization_state_create_info.cullMode = VK_CULL_MODE_BACK_BIT;
            rasterization_state_create_info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
            rasterization_state_create_info.depthBiasEnable = VK_FALSE;
            rasterization_state_create_info.depthBiasConstantFactor = 0.0f;
            rasterization_state_create_info.depthBiasClamp = 0.0f;
            rasterization_state_create_info.depthBiasSlopeFactor = 0.0f;

            VkPipelineMultisampleStateCreateInfo multisample_state_create_info{};
            multisample_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
            multisample_state_create_info.sampleShadingEnable = VK_FALSE;
            multisample_state_create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

            VkPipelineColorBlendAttachmentState color_blend_Attachments[1] = {};
            color_blend_Attachments[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
            color_blend_Attachments[0].blendEnable = VK_FALSE;
            color_blend_Attachments[0].srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
            color_blend_Attachments[0].dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
            color_blend_Attachments[0].colorBlendOp = VK_BLEND_OP_ADD;
            color_blend_Attachments[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            color_blend_Attachments[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
            color_blend_Attachments[0].alphaBlendOp = VK_BLEND_OP_ADD;

            VkPipelineColorBlendStateCreateInfo color_blend_state_create_info = {};
            color_blend_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
            color_blend_state_create_info.logicOpEnable = VK_FALSE;
            color_blend_state_create_info.logicOp = VK_LOGIC_OP_COPY;
            color_blend_state_create_info.attachmentCount =
                sizeof(color_blend_Attachments) / sizeof(color_blend_Attachments[0]);
            color_blend_state_create_info.pAttachments = &color_blend_Attachments[0];
            color_blend_state_create_info.blendConstants[0] = 0.0f;
            color_blend_state_create_info.blendConstants[1] = 0.0f;
            color_blend_state_create_info.blendConstants[2] = 0.0f;
            color_blend_state_create_info.blendConstants[3] = 0.0f;

            VkPipelineDepthStencilStateCreateInfo depth_stencil_create_info{};
            depth_stencil_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
            depth_stencil_create_info.depthTestEnable = VK_TRUE;
            depth_stencil_create_info.depthWriteEnable = VK_TRUE;
            depth_stencil_create_info.depthCompareOp = VK_COMPARE_OP_LESS;
            depth_stencil_create_info.depthBoundsTestEnable = VK_FALSE;
            depth_stencil_create_info.stencilTestEnable = VK_FALSE;

            VkDynamicState                   dynamic_states[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
            VkPipelineDynamicStateCreateInfo dynamic_state_create_info{};
            dynamic_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
            dynamic_state_create_info.dynamicStateCount = 2;
            dynamic_state_create_info.pDynamicStates = dynamic_states;

            VkGraphicsPipelineCreateInfo pipelineInfo{};
            pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
            pipelineInfo.stageCount = 2;
            pipelineInfo.pStages = shader_stages;
            pipelineInfo.pVertexInputState = &vertex_input_state_create_info;
            pipelineInfo.pInputAssemblyState = &input_assembly_create_info;
            pipelineInfo.pViewportState = &viewport_state_create_info;
            pipelineInfo.pRasterizationState = &rasterization_state_create_info;
            pipelineInfo.pMultisampleState = &multisample_state_create_info;
            pipelineInfo.pColorBlendState = &color_blend_state_create_info;
            pipelineInfo.pDepthStencilState = &depth_stencil_create_info;
            pipelineInfo.layout = m_RenderPipelines[_render_pipeline_type_skybox].Layout;
            pipelineInfo.renderPass = m_Framebuffer.RenderPass;
            pipelineInfo.subpass = _main_camera_subpass_forward_lighting;
            pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
            pipelineInfo.pDynamicState = &dynamic_state_create_info;

            if (vkCreateGraphicsPipelines(m_VulkanRhi->m_Device,
                VK_NULL_HANDLE,
                1,
                &pipelineInfo,
                nullptr,
                &m_RenderPipelines[_render_pipeline_type_skybox].Pipeline) != VK_SUCCESS)
            {
                throw std::runtime_error("create skybox graphics pipeline");
            }

            vkDestroyShaderModule(m_VulkanRhi->m_Device, vert_shader_module, nullptr);
            vkDestroyShaderModule(m_VulkanRhi->m_Device, frag_shader_module, nullptr);
        }

        // draw axis
        {
            VkDescriptorSetLayout      descriptorset_layouts[1] = { m_DescriptorInfos[_axis].Layout };
            VkPipelineLayoutCreateInfo pipeline_layout_create_info{};
            pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            pipeline_layout_create_info.setLayoutCount = 1;
            pipeline_layout_create_info.pSetLayouts = descriptorset_layouts;

            if (vkCreatePipelineLayout(m_VulkanRhi->m_Device,
                &pipeline_layout_create_info,
                nullptr,
                &m_RenderPipelines[_render_pipeline_type_axis].Layout) != VK_SUCCESS)
            {
                throw std::runtime_error("create axis pipeline layout");
            }

            VkShaderModule vert_shader_module = VulkanUtil::CreateShaderModule(m_VulkanRhi->m_Device, AXIS_VERT);
            VkShaderModule frag_shader_module = VulkanUtil::CreateShaderModule(m_VulkanRhi->m_Device, AXIS_FRAG);

            VkPipelineShaderStageCreateInfo vert_pipeline_shader_stage_create_info{};
            vert_pipeline_shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            vert_pipeline_shader_stage_create_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
            vert_pipeline_shader_stage_create_info.module = vert_shader_module;
            vert_pipeline_shader_stage_create_info.pName = "main";
            // vert_pipeline_shader_stage_create_info.pSpecializationInfo

            VkPipelineShaderStageCreateInfo frag_pipeline_shader_stage_create_info{};
            frag_pipeline_shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            frag_pipeline_shader_stage_create_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            frag_pipeline_shader_stage_create_info.module = frag_shader_module;
            frag_pipeline_shader_stage_create_info.pName = "main";

            VkPipelineShaderStageCreateInfo shader_stages[] = { vert_pipeline_shader_stage_create_info,
                                                               frag_pipeline_shader_stage_create_info };

            auto                                 vertex_binding_descriptions = MeshVertex::getBindingDescriptions();
            auto                                 vertex_attribute_descriptions = MeshVertex::getAttributeDescriptions();
            VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info{};
            vertex_input_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            vertex_input_state_create_info.vertexBindingDescriptionCount = vertex_binding_descriptions.size();
            vertex_input_state_create_info.pVertexBindingDescriptions = &vertex_binding_descriptions[0];
            vertex_input_state_create_info.vertexAttributeDescriptionCount = vertex_attribute_descriptions.size();
            vertex_input_state_create_info.pVertexAttributeDescriptions = &vertex_attribute_descriptions[0];

            VkPipelineInputAssemblyStateCreateInfo input_assembly_create_info{};
            input_assembly_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
            input_assembly_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            input_assembly_create_info.primitiveRestartEnable = VK_FALSE;

            VkPipelineViewportStateCreateInfo viewport_state_create_info{};
            viewport_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
            viewport_state_create_info.viewportCount = 1;
            viewport_state_create_info.pViewports = &m_VulkanRhi->m_Viewport;
            viewport_state_create_info.scissorCount = 1;
            viewport_state_create_info.pScissors = &m_VulkanRhi->m_Scissor;

            VkPipelineRasterizationStateCreateInfo rasterization_state_create_info{};
            rasterization_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
            rasterization_state_create_info.depthClampEnable = VK_FALSE;
            rasterization_state_create_info.rasterizerDiscardEnable = VK_FALSE;
            rasterization_state_create_info.polygonMode = VK_POLYGON_MODE_FILL;
            rasterization_state_create_info.lineWidth = 1.0f;
            rasterization_state_create_info.cullMode = VK_CULL_MODE_NONE;
            rasterization_state_create_info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
            rasterization_state_create_info.depthBiasEnable = VK_FALSE;
            rasterization_state_create_info.depthBiasConstantFactor = 0.0f;
            rasterization_state_create_info.depthBiasClamp = 0.0f;
            rasterization_state_create_info.depthBiasSlopeFactor = 0.0f;

            VkPipelineMultisampleStateCreateInfo multisample_state_create_info{};
            multisample_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
            multisample_state_create_info.sampleShadingEnable = VK_FALSE;
            multisample_state_create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

            VkPipelineColorBlendAttachmentState color_blend_attachment_state{};
            color_blend_attachment_state.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
            color_blend_attachment_state.blendEnable = VK_FALSE;
            color_blend_attachment_state.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
            color_blend_attachment_state.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
            color_blend_attachment_state.colorBlendOp = VK_BLEND_OP_ADD;
            color_blend_attachment_state.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            color_blend_attachment_state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
            color_blend_attachment_state.alphaBlendOp = VK_BLEND_OP_ADD;

            VkPipelineColorBlendStateCreateInfo color_blend_state_create_info{};
            color_blend_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
            color_blend_state_create_info.logicOpEnable = VK_FALSE;
            color_blend_state_create_info.logicOp = VK_LOGIC_OP_COPY;
            color_blend_state_create_info.attachmentCount = 1;
            color_blend_state_create_info.pAttachments = &color_blend_attachment_state;
            color_blend_state_create_info.blendConstants[0] = 0.0f;
            color_blend_state_create_info.blendConstants[1] = 0.0f;
            color_blend_state_create_info.blendConstants[2] = 0.0f;
            color_blend_state_create_info.blendConstants[3] = 0.0f;

            VkPipelineDepthStencilStateCreateInfo depth_stencil_create_info{};
            depth_stencil_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
            depth_stencil_create_info.depthTestEnable = VK_FALSE;
            depth_stencil_create_info.depthWriteEnable = VK_FALSE;
            depth_stencil_create_info.depthCompareOp = VK_COMPARE_OP_LESS;
            depth_stencil_create_info.depthBoundsTestEnable = VK_FALSE;
            depth_stencil_create_info.stencilTestEnable = VK_FALSE;

            VkDynamicState                   dynamic_states[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
            VkPipelineDynamicStateCreateInfo dynamic_state_create_info{};
            dynamic_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
            dynamic_state_create_info.dynamicStateCount = 2;
            dynamic_state_create_info.pDynamicStates = dynamic_states;

            VkGraphicsPipelineCreateInfo pipelineInfo{};
            pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
            pipelineInfo.stageCount = 2;
            pipelineInfo.pStages = shader_stages;
            pipelineInfo.pVertexInputState = &vertex_input_state_create_info;
            pipelineInfo.pInputAssemblyState = &input_assembly_create_info;
            pipelineInfo.pViewportState = &viewport_state_create_info;
            pipelineInfo.pRasterizationState = &rasterization_state_create_info;
            pipelineInfo.pMultisampleState = &multisample_state_create_info;
            pipelineInfo.pColorBlendState = &color_blend_state_create_info;
            pipelineInfo.pDepthStencilState = &depth_stencil_create_info;
            pipelineInfo.layout = m_RenderPipelines[_render_pipeline_type_axis].Layout;
            pipelineInfo.renderPass = m_Framebuffer.RenderPass;
            pipelineInfo.subpass = _main_camera_subpass_ui;
            pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
            pipelineInfo.pDynamicState = &dynamic_state_create_info;

            if (vkCreateGraphicsPipelines(m_VulkanRhi->m_Device,
                VK_NULL_HANDLE,
                1,
                &pipelineInfo,
                nullptr,
                &m_RenderPipelines[_render_pipeline_type_axis].Pipeline) != VK_SUCCESS)
            {
                throw std::runtime_error("create axis graphics pipeline");
            }

            vkDestroyShaderModule(m_VulkanRhi->m_Device, vert_shader_module, nullptr);
            vkDestroyShaderModule(m_VulkanRhi->m_Device, frag_shader_module, nullptr);
        }
	}
	void MainCameraPass::SetupDescriptorSet()
	{
       SetupModelGlobalDescriptorSet();
       SetupSkyboxDescriptorSet();
       SetupAxisDescriptorSet();
       SetupGbufferLightingDescriptorSet();  
	}
	void MainCameraPass::SetupFramebufferDescriptorSet()
	{
        VkDescriptorImageInfo gbuffer_normal_input_attachment_info = {};
        gbuffer_normal_input_attachment_info.sampler =
            VulkanUtil::GetOrCreateNearestSampler(m_VulkanRhi->m_PhysicalDevice, m_VulkanRhi->m_Device);
        gbuffer_normal_input_attachment_info.imageView = m_Framebuffer.Attachments[_main_camera_pass_gbuffer_a].View;
        gbuffer_normal_input_attachment_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkDescriptorImageInfo gbuffer_metallic_roughness_shadingmodeid_input_attachment_info = {};
        gbuffer_metallic_roughness_shadingmodeid_input_attachment_info.sampler =
            VulkanUtil::GetOrCreateNearestSampler(m_VulkanRhi->m_PhysicalDevice, m_VulkanRhi->m_Device);
        gbuffer_metallic_roughness_shadingmodeid_input_attachment_info.imageView =
            m_Framebuffer.Attachments[_main_camera_pass_gbuffer_b].View;
        gbuffer_metallic_roughness_shadingmodeid_input_attachment_info.imageLayout =
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkDescriptorImageInfo gbuffer_albedo_input_attachment_info = {};
        gbuffer_albedo_input_attachment_info.sampler =
            VulkanUtil::GetOrCreateNearestSampler(m_VulkanRhi->m_PhysicalDevice, m_VulkanRhi->m_Device);
        gbuffer_albedo_input_attachment_info.imageView = m_Framebuffer.Attachments[_main_camera_pass_gbuffer_c].View;
        gbuffer_albedo_input_attachment_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkDescriptorImageInfo depth_input_attachment_info = {};
        depth_input_attachment_info.sampler =
            VulkanUtil::GetOrCreateNearestSampler(m_VulkanRhi->m_PhysicalDevice, m_VulkanRhi->m_Device);
        depth_input_attachment_info.imageView = m_VulkanRhi->m_DepthImageView;
        depth_input_attachment_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkWriteDescriptorSet deferred_lighting_descriptor_writes_info[4];

        VkWriteDescriptorSet& gbuffer_normal_descriptor_input_attachment_write_info =
            deferred_lighting_descriptor_writes_info[0];
        gbuffer_normal_descriptor_input_attachment_write_info.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        gbuffer_normal_descriptor_input_attachment_write_info.pNext = NULL;
        gbuffer_normal_descriptor_input_attachment_write_info.dstSet =
            m_DescriptorInfos[_deferred_lighting].DescriptorSet;
        gbuffer_normal_descriptor_input_attachment_write_info.dstBinding = 0;
        gbuffer_normal_descriptor_input_attachment_write_info.dstArrayElement = 0;
        gbuffer_normal_descriptor_input_attachment_write_info.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
        gbuffer_normal_descriptor_input_attachment_write_info.descriptorCount = 1;
        gbuffer_normal_descriptor_input_attachment_write_info.pImageInfo = &gbuffer_normal_input_attachment_info;

        VkWriteDescriptorSet& gbuffer_metallic_roughness_shadingmodeid_descriptor_input_attachment_write_info =
            deferred_lighting_descriptor_writes_info[1];
        gbuffer_metallic_roughness_shadingmodeid_descriptor_input_attachment_write_info.sType =
            VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        gbuffer_metallic_roughness_shadingmodeid_descriptor_input_attachment_write_info.pNext = NULL;
        gbuffer_metallic_roughness_shadingmodeid_descriptor_input_attachment_write_info.dstSet =
            m_DescriptorInfos[_deferred_lighting].DescriptorSet;
        gbuffer_metallic_roughness_shadingmodeid_descriptor_input_attachment_write_info.dstBinding = 1;
        gbuffer_metallic_roughness_shadingmodeid_descriptor_input_attachment_write_info.dstArrayElement = 0;
        gbuffer_metallic_roughness_shadingmodeid_descriptor_input_attachment_write_info.descriptorType =
            VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
        gbuffer_metallic_roughness_shadingmodeid_descriptor_input_attachment_write_info.descriptorCount = 1;
        gbuffer_metallic_roughness_shadingmodeid_descriptor_input_attachment_write_info.pImageInfo =
            &gbuffer_metallic_roughness_shadingmodeid_input_attachment_info;

        VkWriteDescriptorSet& gbuffer_albedo_descriptor_input_attachment_write_info =
            deferred_lighting_descriptor_writes_info[2];
        gbuffer_albedo_descriptor_input_attachment_write_info.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        gbuffer_albedo_descriptor_input_attachment_write_info.pNext = NULL;
        gbuffer_albedo_descriptor_input_attachment_write_info.dstSet =
            m_DescriptorInfos[_deferred_lighting].DescriptorSet;
        gbuffer_albedo_descriptor_input_attachment_write_info.dstBinding = 2;
        gbuffer_albedo_descriptor_input_attachment_write_info.dstArrayElement = 0;
        gbuffer_albedo_descriptor_input_attachment_write_info.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
        gbuffer_albedo_descriptor_input_attachment_write_info.descriptorCount = 1;
        gbuffer_albedo_descriptor_input_attachment_write_info.pImageInfo = &gbuffer_albedo_input_attachment_info;

        VkWriteDescriptorSet& depth_descriptor_input_attachment_write_info =
            deferred_lighting_descriptor_writes_info[3];
        depth_descriptor_input_attachment_write_info.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        depth_descriptor_input_attachment_write_info.pNext = NULL;
        depth_descriptor_input_attachment_write_info.dstSet = m_DescriptorInfos[_deferred_lighting].DescriptorSet;
        depth_descriptor_input_attachment_write_info.dstBinding = 3;
        depth_descriptor_input_attachment_write_info.dstArrayElement = 0;
        depth_descriptor_input_attachment_write_info.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
        depth_descriptor_input_attachment_write_info.descriptorCount = 1;
        depth_descriptor_input_attachment_write_info.pImageInfo = &depth_input_attachment_info;

        vkUpdateDescriptorSets(m_VulkanRhi->m_Device,
            sizeof(deferred_lighting_descriptor_writes_info) /
            sizeof(deferred_lighting_descriptor_writes_info[0]),
            deferred_lighting_descriptor_writes_info,
            0,
            NULL);
	}
	void MainCameraPass::SetupSwapchainFramebuffers()
	{
        m_SwapchainFramebuffers.resize(m_VulkanRhi->m_SwapchainImageviews.size());
        
        // create frame buffer for every imageview
        for (size_t i = 0; i < m_VulkanRhi->m_SwapchainImageviews.size(); i++)
        {
            VkImageView framebuffer_Attachments_for_image_view[_main_camera_pass_attachment_count] = {
                m_Framebuffer.Attachments[_main_camera_pass_gbuffer_a].View,
                m_Framebuffer.Attachments[_main_camera_pass_gbuffer_b].View,
                m_Framebuffer.Attachments[_main_camera_pass_gbuffer_c].View,
                m_Framebuffer.Attachments[_main_camera_pass_backup_buffer_odd].View,
                m_Framebuffer.Attachments[_main_camera_pass_backup_buffer_even].View,
                m_Framebuffer.Attachments[_main_camera_pass_post_process_buffer_odd].View,
                m_Framebuffer.Attachments[_main_camera_pass_post_process_buffer_even].View,
                m_VulkanRhi->m_DepthImageView,
                m_VulkanRhi->m_SwapchainImageviews[i] };

            VkFramebufferCreateInfo framebuffer_create_info{};
            framebuffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebuffer_create_info.flags = 0U;
            framebuffer_create_info.renderPass = m_Framebuffer.RenderPass;
            framebuffer_create_info.attachmentCount =
                (sizeof(framebuffer_Attachments_for_image_view) / sizeof(framebuffer_Attachments_for_image_view[0]));
            framebuffer_create_info.pAttachments = framebuffer_Attachments_for_image_view;
            framebuffer_create_info.width = m_VulkanRhi->m_SwapchainExtent.width;
            framebuffer_create_info.height = m_VulkanRhi->m_SwapchainExtent.height;
            framebuffer_create_info.layers = 1;

            if (vkCreateFramebuffer(
                m_VulkanRhi->m_Device, &framebuffer_create_info, nullptr, &m_SwapchainFramebuffers[i]) !=
                VK_SUCCESS)
            {
                throw std::runtime_error("create main camera framebuffer");
            }
        }
	}
    void MainCameraPass::SetupModelGlobalDescriptorSet()
    {
        // update common model's global descriptor set
        VkDescriptorSetAllocateInfo mesh_global_descriptor_set_alloc_info;
        mesh_global_descriptor_set_alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        mesh_global_descriptor_set_alloc_info.pNext = NULL;
        mesh_global_descriptor_set_alloc_info.descriptorPool = m_VulkanRhi->m_DescriptorPool;
        mesh_global_descriptor_set_alloc_info.descriptorSetCount = 1;
        mesh_global_descriptor_set_alloc_info.pSetLayouts = &m_DescriptorInfos[_mesh_global].Layout;

        if (VK_SUCCESS != vkAllocateDescriptorSets(m_VulkanRhi->m_Device,
            &mesh_global_descriptor_set_alloc_info,
            &m_DescriptorInfos[_mesh_global].DescriptorSet))
        {
            throw std::runtime_error("allocate mesh global descriptor set");
        }

        VkDescriptorBufferInfo mesh_perframe_storage_buffer_info = {};
        // this offset plus dynamic_offset should not be greater than the size of the buffer
        mesh_perframe_storage_buffer_info.offset = 0;
        // the range means the size actually used by the shader per draw call
        mesh_perframe_storage_buffer_info.range = sizeof(MeshPerframeStorageBufferObject);
        mesh_perframe_storage_buffer_info.buffer = m_GlobalRenderResource->_storage_buffer._global_upload_ringbuffer;
        assert(mesh_perframe_storage_buffer_info.range <
            m_GlobalRenderResource->_storage_buffer._max_storage_buffer_range);

        VkDescriptorBufferInfo mesh_perdrawcall_storage_buffer_info = {};
        mesh_perdrawcall_storage_buffer_info.offset = 0;
        mesh_perdrawcall_storage_buffer_info.range = sizeof(MeshPerdrawcallStorageBufferObject);
        mesh_perdrawcall_storage_buffer_info.buffer =
            m_GlobalRenderResource->_storage_buffer._global_upload_ringbuffer;
        assert(mesh_perdrawcall_storage_buffer_info.range <
            m_GlobalRenderResource->_storage_buffer._max_storage_buffer_range);

        VkDescriptorBufferInfo mesh_per_drawcall_vertex_blending_storage_buffer_info = {};
        mesh_per_drawcall_vertex_blending_storage_buffer_info.offset = 0;
        mesh_per_drawcall_vertex_blending_storage_buffer_info.range =
            sizeof(MeshPerdrawcallVertexBlendingStorageBufferObject);
        mesh_per_drawcall_vertex_blending_storage_buffer_info.buffer =
            m_GlobalRenderResource->_storage_buffer._global_upload_ringbuffer;
        assert(mesh_per_drawcall_vertex_blending_storage_buffer_info.range <
            m_GlobalRenderResource->_storage_buffer._max_storage_buffer_range);

        VkDescriptorImageInfo brdf_texture_image_info = {};
        brdf_texture_image_info.sampler = m_GlobalRenderResource->_ibl_resource._brdfLUT_texture_sampler;
        brdf_texture_image_info.imageView = m_GlobalRenderResource->_ibl_resource._brdfLUT_texture_image_view;
        brdf_texture_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkDescriptorImageInfo irradiance_texture_image_info = {};
        irradiance_texture_image_info.sampler = m_GlobalRenderResource->_ibl_resource._irradiance_texture_sampler;
        irradiance_texture_image_info.imageView =
            m_GlobalRenderResource->_ibl_resource._irradiance_texture_image_view;
        irradiance_texture_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkDescriptorImageInfo specular_texture_image_info{};
        specular_texture_image_info.sampler = m_GlobalRenderResource->_ibl_resource._specular_texture_sampler;
        specular_texture_image_info.imageView = m_GlobalRenderResource->_ibl_resource._specular_texture_image_view;
        specular_texture_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkDescriptorImageInfo point_light_shadow_texture_image_info{};
        point_light_shadow_texture_image_info.sampler =
            VulkanUtil::GetOrCreateNearestSampler(m_VulkanRhi->m_PhysicalDevice, m_VulkanRhi->m_Device);
        point_light_shadow_texture_image_info.imageView = m_point_light_shadow_color_image_view;
        point_light_shadow_texture_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkDescriptorImageInfo directional_light_shadow_texture_image_info{};
        directional_light_shadow_texture_image_info.sampler =
            VulkanUtil::GetOrCreateNearestSampler(m_VulkanRhi->m_PhysicalDevice, m_VulkanRhi->m_Device);
        directional_light_shadow_texture_image_info.imageView = m_directional_light_shadow_color_image_view;
        directional_light_shadow_texture_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkWriteDescriptorSet mesh_descriptor_writes_info[8];

        mesh_descriptor_writes_info[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        mesh_descriptor_writes_info[0].pNext = NULL;
        mesh_descriptor_writes_info[0].dstSet = m_DescriptorInfos[_mesh_global].DescriptorSet;
        mesh_descriptor_writes_info[0].dstBinding = 0;
        mesh_descriptor_writes_info[0].dstArrayElement = 0;
        mesh_descriptor_writes_info[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
        mesh_descriptor_writes_info[0].descriptorCount = 1;
        mesh_descriptor_writes_info[0].pBufferInfo = &mesh_perframe_storage_buffer_info;

        mesh_descriptor_writes_info[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        mesh_descriptor_writes_info[1].pNext = NULL;
        mesh_descriptor_writes_info[1].dstSet = m_DescriptorInfos[_mesh_global].DescriptorSet;
        mesh_descriptor_writes_info[1].dstBinding = 1;
        mesh_descriptor_writes_info[1].dstArrayElement = 0;
        mesh_descriptor_writes_info[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
        mesh_descriptor_writes_info[1].descriptorCount = 1;
        mesh_descriptor_writes_info[1].pBufferInfo = &mesh_perdrawcall_storage_buffer_info;

        mesh_descriptor_writes_info[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        mesh_descriptor_writes_info[2].pNext = NULL;
        mesh_descriptor_writes_info[2].dstSet = m_DescriptorInfos[_mesh_global].DescriptorSet;
        mesh_descriptor_writes_info[2].dstBinding = 2;
        mesh_descriptor_writes_info[2].dstArrayElement = 0;
        mesh_descriptor_writes_info[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
        mesh_descriptor_writes_info[2].descriptorCount = 1;
        mesh_descriptor_writes_info[2].pBufferInfo = &mesh_per_drawcall_vertex_blending_storage_buffer_info;

        mesh_descriptor_writes_info[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        mesh_descriptor_writes_info[3].pNext = NULL;
        mesh_descriptor_writes_info[3].dstSet = m_DescriptorInfos[_mesh_global].DescriptorSet;
        mesh_descriptor_writes_info[3].dstBinding = 3;
        mesh_descriptor_writes_info[3].dstArrayElement = 0;
        mesh_descriptor_writes_info[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        mesh_descriptor_writes_info[3].descriptorCount = 1;
        mesh_descriptor_writes_info[3].pImageInfo = &brdf_texture_image_info;

        mesh_descriptor_writes_info[4] = mesh_descriptor_writes_info[3];
        mesh_descriptor_writes_info[4].dstBinding = 4;
        mesh_descriptor_writes_info[4].pImageInfo = &irradiance_texture_image_info;

        mesh_descriptor_writes_info[5] = mesh_descriptor_writes_info[3];
        mesh_descriptor_writes_info[5].dstBinding = 5;
        mesh_descriptor_writes_info[5].pImageInfo = &specular_texture_image_info;

        mesh_descriptor_writes_info[6] = mesh_descriptor_writes_info[3];
        mesh_descriptor_writes_info[6].dstBinding = 6;
        mesh_descriptor_writes_info[6].pImageInfo = &point_light_shadow_texture_image_info;

        mesh_descriptor_writes_info[7] = mesh_descriptor_writes_info[3];
        mesh_descriptor_writes_info[7].dstBinding = 7;
        mesh_descriptor_writes_info[7].pImageInfo = &directional_light_shadow_texture_image_info;

        vkUpdateDescriptorSets(m_VulkanRhi->m_Device,
            sizeof(mesh_descriptor_writes_info) / sizeof(mesh_descriptor_writes_info[0]),
            mesh_descriptor_writes_info,
            0,
            NULL);
    }
    void MainCameraPass::SetupSkyboxDescriptorSet()
    {
        VkDescriptorSetAllocateInfo skybox_descriptor_set_alloc_info;
        skybox_descriptor_set_alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        skybox_descriptor_set_alloc_info.pNext = NULL;
        skybox_descriptor_set_alloc_info.descriptorPool = m_VulkanRhi->m_DescriptorPool;
        skybox_descriptor_set_alloc_info.descriptorSetCount = 1;
        skybox_descriptor_set_alloc_info.pSetLayouts = &m_DescriptorInfos[_skybox].Layout;

        if (VK_SUCCESS != vkAllocateDescriptorSets(m_VulkanRhi->m_Device,
            &skybox_descriptor_set_alloc_info,
            &m_DescriptorInfos[_skybox].DescriptorSet))
        {
            throw std::runtime_error("allocate skybox descriptor set");
        }

        VkDescriptorBufferInfo mesh_perframe_storage_buffer_info = {};
        mesh_perframe_storage_buffer_info.offset = 0;
        mesh_perframe_storage_buffer_info.range = sizeof(MeshPerframeStorageBufferObject);
        mesh_perframe_storage_buffer_info.buffer = m_GlobalRenderResource->_storage_buffer._global_upload_ringbuffer;
        assert(mesh_perframe_storage_buffer_info.range <
            m_GlobalRenderResource->_storage_buffer._max_storage_buffer_range);

        VkDescriptorImageInfo specular_texture_image_info = {};
        specular_texture_image_info.sampler = m_GlobalRenderResource->_ibl_resource._specular_texture_sampler;
        specular_texture_image_info.imageView = m_GlobalRenderResource->_ibl_resource._specular_texture_image_view;
        specular_texture_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkWriteDescriptorSet skybox_descriptor_writes_info[2];

        skybox_descriptor_writes_info[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        skybox_descriptor_writes_info[0].pNext = NULL;
        skybox_descriptor_writes_info[0].dstSet = m_DescriptorInfos[_skybox].DescriptorSet;
        skybox_descriptor_writes_info[0].dstBinding = 0;
        skybox_descriptor_writes_info[0].dstArrayElement = 0;
        skybox_descriptor_writes_info[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
        skybox_descriptor_writes_info[0].descriptorCount = 1;
        skybox_descriptor_writes_info[0].pBufferInfo = &mesh_perframe_storage_buffer_info;

        skybox_descriptor_writes_info[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        skybox_descriptor_writes_info[1].pNext = NULL;
        skybox_descriptor_writes_info[1].dstSet = m_DescriptorInfos[_skybox].DescriptorSet;
        skybox_descriptor_writes_info[1].dstBinding = 1;
        skybox_descriptor_writes_info[1].dstArrayElement = 0;
        skybox_descriptor_writes_info[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        skybox_descriptor_writes_info[1].descriptorCount = 1;
        skybox_descriptor_writes_info[1].pImageInfo = &specular_texture_image_info;

        vkUpdateDescriptorSets(m_VulkanRhi->m_Device, 2, skybox_descriptor_writes_info, 0, NULL);
    }
    void MainCameraPass::SetupAxisDescriptorSet()
    {
        VkDescriptorSetAllocateInfo axis_descriptor_set_alloc_info;
        axis_descriptor_set_alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        axis_descriptor_set_alloc_info.pNext = NULL;
        axis_descriptor_set_alloc_info.descriptorPool = m_VulkanRhi->m_DescriptorPool;
        axis_descriptor_set_alloc_info.descriptorSetCount = 1;
        axis_descriptor_set_alloc_info.pSetLayouts = &m_DescriptorInfos[_axis].Layout;

        if (VK_SUCCESS != vkAllocateDescriptorSets(m_VulkanRhi->m_Device,
            &axis_descriptor_set_alloc_info,
            &m_DescriptorInfos[_axis].DescriptorSet))
        {
            throw std::runtime_error("allocate axis descriptor set");
        }

        VkDescriptorBufferInfo mesh_perframe_storage_buffer_info = {};
        mesh_perframe_storage_buffer_info.offset = 0;
        mesh_perframe_storage_buffer_info.range = sizeof(MeshPerframeStorageBufferObject);
        mesh_perframe_storage_buffer_info.buffer = m_GlobalRenderResource->_storage_buffer._global_upload_ringbuffer;
        assert(mesh_perframe_storage_buffer_info.range <
            m_GlobalRenderResource->_storage_buffer._max_storage_buffer_range);

        VkDescriptorBufferInfo axis_storage_buffer_info = {};
        axis_storage_buffer_info.offset = 0;
        axis_storage_buffer_info.range = sizeof(AxisStorageBufferObject);
        axis_storage_buffer_info.buffer = m_GlobalRenderResource->_storage_buffer._axis_inefficient_storage_buffer;

        VkWriteDescriptorSet axis_descriptor_writes_info[2];

        axis_descriptor_writes_info[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        axis_descriptor_writes_info[0].pNext = NULL;
        axis_descriptor_writes_info[0].dstSet = m_DescriptorInfos[_axis].DescriptorSet;
        axis_descriptor_writes_info[0].dstBinding = 0;
        axis_descriptor_writes_info[0].dstArrayElement = 0;
        axis_descriptor_writes_info[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
        axis_descriptor_writes_info[0].descriptorCount = 1;
        axis_descriptor_writes_info[0].pBufferInfo = &mesh_perframe_storage_buffer_info;

        axis_descriptor_writes_info[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        axis_descriptor_writes_info[1].pNext = NULL;
        axis_descriptor_writes_info[1].dstSet = m_DescriptorInfos[_axis].DescriptorSet;
        axis_descriptor_writes_info[1].dstBinding = 1;
        axis_descriptor_writes_info[1].dstArrayElement = 0;
        axis_descriptor_writes_info[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        axis_descriptor_writes_info[1].descriptorCount = 1;
        axis_descriptor_writes_info[1].pBufferInfo = &axis_storage_buffer_info;

        vkUpdateDescriptorSets(m_VulkanRhi->m_Device,
            (uint32_t)(sizeof(axis_descriptor_writes_info) / sizeof(axis_descriptor_writes_info[0])),
            axis_descriptor_writes_info,
            0,
            NULL);
    }
    void MainCameraPass::SetupGbufferLightingDescriptorSet()
    {
        VkDescriptorSetAllocateInfo gbuffer_light_global_descriptor_set_alloc_info;
        gbuffer_light_global_descriptor_set_alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        gbuffer_light_global_descriptor_set_alloc_info.pNext = NULL;
        gbuffer_light_global_descriptor_set_alloc_info.descriptorPool = m_VulkanRhi->m_DescriptorPool;
        gbuffer_light_global_descriptor_set_alloc_info.descriptorSetCount = 1;
        gbuffer_light_global_descriptor_set_alloc_info.pSetLayouts = &m_DescriptorInfos[_deferred_lighting].Layout;

        if (VK_SUCCESS != vkAllocateDescriptorSets(m_VulkanRhi->m_Device,
            &gbuffer_light_global_descriptor_set_alloc_info,
            &m_DescriptorInfos[_deferred_lighting].DescriptorSet))
        {
            throw std::runtime_error("allocate gbuffer light global descriptor set");
        }
    }
}
