#include "pch.h"
#include "RuntimeGlobalContext.h"

#include "Runtime/Function/Render/WindowSystem.h"
#include "Runtime/Function/Render/RenderSystem.h"

namespace BE
{
	RuntimeGlobalContext g_RuntimeGlobalContext;

	void RuntimeGlobalContext::StartSystems(const std::string& configFilePath)
	{
		m_WindowSystem = std::make_shared<WindowSystem>();
		WindowCreateInfo windowCreateInfo;
		m_WindowSystem->Initialize(windowCreateInfo);

		m_RenderSystem = std::make_shared<RenderSystem>();
		RenderSystemInitInfo renderInitInfo;
		renderInitInfo.WindowSystem = m_WindowSystem;
		m_RenderSystem->Initialize(renderInitInfo);
	}
	void RuntimeGlobalContext::ShutdownSystems()
	{
		m_RenderSystem.reset();
		m_WindowSystem.reset();
	}
}


