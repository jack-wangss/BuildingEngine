#include "pch.h"
#include "RenderPipelineBase.h"
namespace BE
{
	void RenderPipelineBase::DeferredRender(std::shared_ptr<Rhi> rhi, std::shared_ptr<RenderResourceBase> renderResource)
	{

	}
	void RenderPipelineBase::InitializeUIRenderBackend(WindowUI* windowUI)
	{
		m_UIPass->InitializeUIRenderBackend(windowUI);
	}
}
