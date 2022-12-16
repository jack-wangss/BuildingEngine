#include "pch.h"
#include "EditorGlobalContext.h"

namespace BE
{
	EditorGlobalContext g_EditorGlobalContext;

	void EditorGlobalContext::Initialize(const EditorGlobalContextInitInfo& info)
	{
		g_EditorGlobalContext.m_WindowSystem = info.WindowSystem;
		g_EditorGlobalContext.m_RenderSystem = info.RenderSystem;
		g_EditorGlobalContext.m_Engine = info.Engine;
	}
	void EditorGlobalContext::Clear()
	{

	}
}
