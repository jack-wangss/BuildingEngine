#include "pch.h"
#include "Engine.h"
#include "Runtime/Function/Global/RuntimeGlobalContext.h"

#include "Runtime/Function/Render/WindowSystem.h"
#include "Runtime/Function/Render/RenderSystem.h"

namespace BE
{
	bool                            g_IsEditorMode{ false };

	void Engine::StartEngine(const std::string& configFilePath)
	{
		// Reflection::TypeMetaRegister::Register();

		g_RuntimeGlobalContext.StartSystems(configFilePath);

		// LOG_INFO("engine start");
	}
	void Engine::ShutdownEngine()
	{
		// LOG_INFO("engine shutdown");

		g_RuntimeGlobalContext.ShutdownSystems();

		// Reflection::TypeMetaRegister::Unregister();
	}
	bool Engine::TickOneFrame(float deltaTime)
	{
		LogicalTick(deltaTime);
		CalculateFPS(deltaTime);

		// single thread
		// exchange data between logic and render contexts
		// g_RuntimeGlobalContext.m_RenderSystem->swapLogicRenderData();

		RendererTick();

		g_RuntimeGlobalContext.m_WindowSystem->PollEvents();


		g_RuntimeGlobalContext.m_WindowSystem->SetTitle(
			std::string("BuildingEngine - " + std::to_string(GetFPS()) + " FPS").c_str());

		const bool shouldWindowClose = g_RuntimeGlobalContext.m_WindowSystem->ShouldClose();
		return !shouldWindowClose;
	}
	void Engine::LogicalTick(float delta_time)
	{
	}
	bool Engine::RendererTick()
	{
		g_RuntimeGlobalContext.m_RenderSystem->Tick();
		return true;
	}
	void Engine::CalculateFPS(float delta_time)
	{
	}
	float Engine::CalculateDeltaTime()
	{
		float deltaTime;
		{
			using namespace std::chrono;

			steady_clock::time_point tick_time_point = steady_clock::now();
			duration<float> time_span = duration_cast<duration<float>>(tick_time_point - m_LastTickTimePoint);
			deltaTime = time_span.count();
			m_LastTickTimePoint = tick_time_point;
		}
		return deltaTime;
	}
}
