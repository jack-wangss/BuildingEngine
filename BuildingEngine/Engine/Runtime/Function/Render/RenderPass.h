#pragma once

#include "Runtime/Function/Render/RenderPassBase.h"
#include "Runtime/Function/Render/RenderResource.h"

namespace BE
{
    class VulkanRhi;


    enum
    {
        _main_camera_pass_gbuffer_a = 0,
        _main_camera_pass_gbuffer_b = 1,
        _main_camera_pass_gbuffer_c = 2,
        _main_camera_pass_backup_buffer_odd = 3,
        _main_camera_pass_backup_buffer_even = 4,
        _main_camera_pass_post_process_buffer_odd = 5,
        _main_camera_pass_post_process_buffer_even = 6,
        _main_camera_pass_depth = 7,
        _main_camera_pass_swap_chain_image = 8,
        _main_camera_pass_custom_attachment_count = 5,
        _main_camera_pass_post_process_attachment_count = 2,
        _main_camera_pass_attachment_count = 9,
    };

    enum
    {
        _main_camera_subpass_basepass = 0,
        _main_camera_subpass_deferred_lighting,
        _main_camera_subpass_forward_lighting,
        _main_camera_subpass_tone_mapping,
        _main_camera_subpass_color_grading,
        _main_camera_subpass_fxaa,
        _main_camera_subpass_ui,
        _main_camera_subpass_combine_ui,
        _main_camera_subpass_count
    };


    class RenderPass : public RenderPassBase
    {
    public:
        struct FrameBufferAttachment
        {
            VkImage        Image;
            VkDeviceMemory Mem;
            VkImageView    View;
            VkFormat       Format;
        };

        struct Framebuffer
        {
            int           Width;
            int           Height;
            VkFramebuffer Framebuffer;
            VkRenderPass  RenderPass;

            std::vector<FrameBufferAttachment> Attachments;
        };

        struct Descriptor
        {
            VkDescriptorSetLayout Layout;
            VkDescriptorSet       DescriptorSet;
        };

        struct RenderPipelineBase
        {
            VkPipelineLayout       Layout;
            VkPipeline             Pipeline;
        };

        GlobalRenderResource* m_GlobalRenderResource{ nullptr };

        //std::vector<Descriptor>         m_descriptor_infos;
        std::vector<RenderPipelineBase> m_RenderPipelines;

        void Initialize(const RenderPassInitInfo* init_info) override;
        void PostInitialize() override;

        virtual void Draw();

        virtual VkRenderPass                       GetRenderPass() const;
        //virtual std::vector<VkImageView>           GetFramebufferImageViews() const;
        //virtual std::vector<VkDescriptorSetLayout> GetDescriptorSetLayouts() const;

        //static VisiableNodes m_visiable_nodes;

    protected:
        std::shared_ptr<VulkanRhi>      m_VulkanRhi{ nullptr };

        std::vector<Descriptor>         m_DescriptorInfos;
        Framebuffer                     m_Framebuffer;
    };
}