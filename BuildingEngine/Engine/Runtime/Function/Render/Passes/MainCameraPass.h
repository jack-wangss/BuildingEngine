#pragma once

#include "Runtime/Function/Render/RenderPass.h"
#include "Runtime/Function/Render/Passes/UIPass.h"

namespace BE
{
    struct MainCameraPassInitInfo : RenderPassInitInfo
    {
        bool EnableFxaa;
    };

	class MainCameraPass : public RenderPass
	{
    public:
        // 1: per mesh layout
        // 2: global layout
        // 3: mesh per material layout
        // 4: sky box layout
        // 5: axis layout
        // 6: billboard type particle layout
        // 7: gbuffer lighting
        enum LayoutType : uint8_t
        {
            _per_mesh = 0,
            _mesh_global,
            _mesh_per_material,
            _skybox,
            _axis,
            _particle,
            _deferred_lighting,
            _layout_type_count
        };

        // 1. model
        // 2. sky box
        // 3. axis
        // 4. billboard type particle
        enum RenderPipeLineType : uint8_t
        {
            _render_pipeline_type_mesh_gbuffer = 0,
            _render_pipeline_type_deferred_lighting,
            _render_pipeline_type_mesh_lighting,
            _render_pipeline_type_skybox,
            _render_pipeline_type_axis,
            _render_pipeline_type_particle,
            _render_pipeline_type_count
        };

        void Initialize(const RenderPassInitInfo* initInfo) override final;

        //void PreparePassData(std::shared_ptr<RenderResourceBase> renderResource) override final;

        void Draw(/*ColorGradingPass& color_grading_pass,
            FXAAPass& fxaa_pass,
            ToneMappingPass& tone_mapping_pass,*/
            UIPass& uiPass/*,
            CombineUIPass& combine_ui_pass,
            ParticlePass& particle_pass,
            uint32_t          current_swapchain_image_index*/);

        VkImageView m_point_light_shadow_color_image_view;
        VkImageView m_directional_light_shadow_color_image_view;

        bool                        m_EnableFxaa{ false };

    private:
        void SetupAttachments();
        void SetupRenderPass();
        void SetupDescriptorSetLayout();
        void SetupPipelines();
        void SetupDescriptorSet();
        void SetupFramebufferDescriptorSet();
        void SetupSwapchainFramebuffers();

        void SetupModelGlobalDescriptorSet();
        void SetupSkyboxDescriptorSet();
        void SetupAxisDescriptorSet();
        void SetupGbufferLightingDescriptorSet();

    private:
        std::vector<VkFramebuffer> m_SwapchainFramebuffers;
	};
}