#include "pch.h"
#include "Editor.h"
#include "EditorUI.h"

#include "EditorGlobalContext.h"

#include "Runtime/Engine.h"
#include "Runtime/Function/Global/RuntimeGlobalContext.h"


namespace BE
{

	Editor::Editor()
	{
	}

	Editor::~Editor()
	{
	}

	void Editor::Initialize(Engine* engine)
	{
		g_IsEditorMode = true;
		m_Engine = engine;

		EditorGlobalContextInitInfo info = { g_RuntimeGlobalContext.m_WindowSystem.get(),
												 g_RuntimeGlobalContext.m_RenderSystem.get(),
												 engine };
		g_EditorGlobalContext.Initialize(info);

		m_UI = std::make_shared<EditorUI>();
	
		WindowUIInitInfo uiInfo = { g_RuntimeGlobalContext.m_WindowSystem,
										 g_RuntimeGlobalContext.m_RenderSystem };
		
		m_UI->Initialize(uiInfo);
	}

	void Editor::Clear()
	{
		g_EditorGlobalContext.Clear();
	}

	void Editor::Run()
	{

		float deltaTime;
		while (true)
		{
			deltaTime = m_Engine->CalculateDeltaTime();

			if (!m_Engine->TickOneFrame(deltaTime))
				return;
		}

	}

}