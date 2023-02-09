#pragma once

namespace BE
{
	class WindowSystem;
	class Rhi;
	class RenderResourceBase;
	class RenderPipelineBase;
	class WindowUI;


	struct RenderSystemInitInfo
	{
		std::shared_ptr<WindowSystem> WindowSystem;
	};

	class RenderSystem
	{
	public:
		RenderSystem() = default;
		~RenderSystem();


		void Initialize(RenderSystemInitInfo init_info);
		void Tick();

		void InitializeUIRenderBackend(WindowUI* windowUI);
	private:
		std::shared_ptr<Rhi>                m_Rhi;
		std::shared_ptr<RenderPipelineBase> m_RenderPipeline;
		std::shared_ptr<RenderResourceBase> m_RenderResource;

	};

}