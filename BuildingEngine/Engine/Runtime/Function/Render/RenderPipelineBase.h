#pragma once
#include "RenderPassBase.h"

namespace BE
{
	class Rhi;
	class RenderResourceBase;
	class WindowUI;

	struct RenderPipelineInitInfo
	{
		bool                                EnableFxaa{ false };
		std::shared_ptr<RenderResourceBase> RenderResource;
	};

	class RenderPipelineBase
	{
		friend class RenderSystem;

	public:
		virtual ~RenderPipelineBase() {}

		virtual void Initialize(RenderPipelineInitInfo initInfo) = 0;
		virtual void DeferredRender(std::shared_ptr<Rhi> rhi, std::shared_ptr<RenderResourceBase> renderResource);
	
		void             InitializeUIRenderBackend(WindowUI* windowUI);
	protected:
		std::shared_ptr<Rhi> m_Rhi;

		std::shared_ptr<RenderPassBase> m_MainCameraPass;
		std::shared_ptr<RenderPassBase> m_UIPass;
	};
}