#include "pch.h"
#include "RenderPipeline.h"

#include "Runtime/Function/Render/Passes/MainCameraPass.h"

namespace BE
{
	void RenderPipeline::Initialize(RenderPipelineInitInfo initInfo)
	{
		m_MainCameraPass = std::make_shared<MainCameraPass>();
		m_UIPass = std::make_shared<UIPass>();

		RenderPassCommonInfo passCommonInfo;
		passCommonInfo.Rhi = m_Rhi;
		passCommonInfo.RenderResource = initInfo.RenderResource;

		m_MainCameraPass->SetCommonInfo(passCommonInfo);
		m_UIPass->SetCommonInfo(passCommonInfo);

		std::shared_ptr<MainCameraPass> main_camera_pass = std::static_pointer_cast<MainCameraPass>(m_MainCameraPass);
		std::shared_ptr<RenderPass> _main_camera_pass = std::static_pointer_cast<RenderPass>(m_MainCameraPass);

		MainCameraPassInitInfo main_camera_init_info;
		main_camera_init_info.EnableFxaa = initInfo.EnableFxaa;
		//main_camera_pass->SetParticlePass(particle_pass);
		m_MainCameraPass->Initialize(&main_camera_init_info);

		UIPassInitInfo uiInitInfo;
		uiInitInfo.RenderPass = _main_camera_pass->GetRenderPass();
		m_UIPass->Initialize(&uiInitInfo);
	}
	void RenderPipeline::DeferredRender(std::shared_ptr<Rhi> rhi, std::shared_ptr<RenderResourceBase> renderResource)
	{
		UIPass& uiPass = *(static_cast<UIPass*>(m_UIPass.get()));
		static_cast<MainCameraPass*>(m_MainCameraPass.get())
			->Draw(/*color_grading_pass,
				fxaa_pass,
				tone_mapping_pass,*/
				uiPass/*,
				combine_ui_pass,
				particle_pass,
				vulkan_rhi->m_current_swapchain_image_index*/);
	}
}
