#pragma once

namespace BE
{
	class WindowSystem;

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
	};

}