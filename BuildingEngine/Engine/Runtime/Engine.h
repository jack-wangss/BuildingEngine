#pragma once

namespace BE
{
	extern bool                            g_IsEditorMode;
	class Engine
	{
		friend	class Editor;

	public:
		void StartEngine(const std::string& configFilePath);
		void ShutdownEngine();

		bool TickOneFrame(float deltaTime);
		int GetFPS() const { return m_Fps; }
	protected:
		void LogicalTick(float deltaTime);
		bool RendererTick();

		void CalculateFPS(float deltaTime);
		/**
		 *  Each frame can only be called once
		 */
		float CalculateDeltaTime();
		
	protected:
		std::chrono::steady_clock::time_point m_LastTickTimePoint{ std::chrono::steady_clock::now() };
		int   m_Fps{ 0 };
	};
}
