#include "pch.h"
#include "RuntimeGlobalContext.h"

#include "Runtime/Function/Render/WindowSystem.h"

namespace BE
{
	RuntimeGlobalContext g_RuntimeGlobalContext;

	void RuntimeGlobalContext::StartSystems(const std::string& configFilePath)
	{
		m_WindowSystem = std::make_shared<WindowSystem>();
		WindowCreateInfo windowCreateInfo;
		m_WindowSystem->Initialize(windowCreateInfo);
	}
	void RuntimeGlobalContext::ShutdownSystems()
	{
		m_WindowSystem.reset();
	}
}


