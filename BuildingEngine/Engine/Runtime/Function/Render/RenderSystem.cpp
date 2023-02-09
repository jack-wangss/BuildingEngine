#include "pch.h"
#include "RenderSystem.h"

#include "Runtime/Function/Render/RenderResource.h"
#include "Runtime/Function/Render/RenderPipeline.h"

#include "Runtime/Function/Render/Rhi/Vulkan/VulkanRhi.h"

namespace BE
{
	RenderSystem::~RenderSystem()
	{
	}
	void RenderSystem::Initialize(RenderSystemInitInfo initInfo)
	{
        // render context initialize
        RhiInitInfo rhiInitInfo;
        rhiInitInfo.WindowSystem = initInfo.WindowSystem;

        m_Rhi = std::make_shared<VulkanRhi>();
        m_Rhi->Initialize(rhiInitInfo);

        m_RenderResource = std::make_shared<RenderResource>();

        // initialize render pipeline
        RenderPipelineInitInfo pipelineInitInfo;
        pipelineInitInfo.EnableFxaa =/* global_rendering_res.m_enable_fxaa*/true;
        pipelineInitInfo.RenderResource = m_RenderResource;

        m_RenderPipeline = std::make_shared<RenderPipeline>();
        m_RenderPipeline->m_Rhi = m_Rhi;
        m_RenderPipeline->Initialize(pipelineInitInfo);

        // descriptor set layout in main camera pass will be used when uploading resource
        //std::static_pointer_cast<RenderResource>(m_RenderResource)->m_mesh_descriptor_set_layout =
        //    &static_cast<RenderPass*>(m_RenderPipeline->m_MainCameraPass.get())
        //    ->m_descriptor_infos[MainCameraPass::LayoutType::_per_mesh]
        //    .layout;
        //std::static_pointer_cast<RenderResource>(m_RenderResource)->m_material_descriptor_set_layout =
        //    &static_cast<RenderPass*>(m_RenderPipeline->m_MainCameraPass.get())
        //    ->m_descriptor_infos[MainCameraPass::LayoutType::_mesh_per_material]
        //    .layout;
	}
	void RenderSystem::Tick()
	{
        //// process swap data between logic and render contexts
        //processSwapData();

        //// prepare render command context
        //m_rhi->prepareContext();

        //// update per-frame buffer
        //m_render_resource->updatePerFrameBuffer(m_render_scene, m_render_camera);

        //// update per-frame visible objects
        //m_render_scene->updateVisibleObjects(std::static_pointer_cast<RenderResource>(m_render_resource),
        //    m_render_camera);

        //// prepare pipeline's render passes data
        //m_render_pipeline->preparePassData(m_render_resource);

        //// render one frame
        //if (m_render_pipeline_type == RENDER_PIPELINE_TYPE::FORWARD_PIPELINE)
        //{
        //    m_render_pipeline->forwardRender(m_rhi, m_render_resource);
        //}
        //else if (m_render_pipeline_type == RENDER_PIPELINE_TYPE::DEFERRED_PIPELINE)
        //{
        m_RenderPipeline->DeferredRender(m_Rhi, m_RenderResource);
        //}
        //else
        //{
        //    LOG_ERROR(__FUNCTION__, "unsupported render pipeline type");
        //}
	}
    void RenderSystem::InitializeUIRenderBackend(WindowUI* windowUI)
    {
        m_RenderPipeline->InitializeUIRenderBackend(windowUI);
    }
}
