#pragma once

namespace BE
{
	class WindowSystem;
	class RenderSystem;

	class RuntimeGlobalContext
	{
	public:
		void StartSystems(const std::string& configFilePath);
		void ShutdownSystems();
	public:
		std::shared_ptr<WindowSystem>    m_WindowSystem;
		std::shared_ptr<RenderSystem>    m_RenderSystem;
	};
	extern RuntimeGlobalContext g_RuntimeGlobalContext;
}