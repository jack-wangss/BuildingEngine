#include "pch.h"
#include "RenderPass.h"

#include "Runtime/Function/Render/RenderResource.h"
#include "Runtime/Function/Render/Rhi/Vulkan/VulkanRhi.h"

namespace BE
{


	void RenderPass::Initialize(const RenderPassInitInfo* initInfo)
	{
		m_VulkanRhi = std::static_pointer_cast<VulkanRhi>(m_Rhi);
		m_GlobalRenderResource =
			&(std::static_pointer_cast<RenderResource>(m_RenderResource)->m_GlobalRenderResource);
	}

	void RenderPass::PostInitialize()
	{
	}

	void RenderPass::Draw()
	{
	}

	VkRenderPass RenderPass::GetRenderPass() const
	{
		return m_Framebuffer.RenderPass;
	}

}
