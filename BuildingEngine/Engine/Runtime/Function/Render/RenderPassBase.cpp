#include "pch.h"
#include "RenderPassBase.h"

namespace BE
{
	void RenderPassBase::PostInitialize()
	{

	}
	void RenderPassBase::SetCommonInfo(RenderPassCommonInfo commonInfo)
	{
		m_Rhi = commonInfo.Rhi;
		m_RenderResource = commonInfo.RenderResource;
	}
	void RenderPassBase::PreparePassData(std::shared_ptr<RenderResourceBase> renderResource)
	{

	}
	void RenderPassBase::InitializeUIRenderBackend(WindowUI* windowUI)
	{

	}
}
