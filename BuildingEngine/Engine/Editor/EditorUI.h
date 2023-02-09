#pragma once


#include "Runtime/Function/UI/WindowUI.h"

namespace BE
{

	class EditorUI : public WindowUI
	{
	public:
		EditorUI();

	public:
		virtual void Initialize(WindowUIInitInfo info) override final;
		virtual void PreRender() override final;

	private:
		void ShowEditorUI();
		void ShowEditorMenu(bool* isShow);
		void ShowEditorWorldObjectsWindow(bool* isShow);
		void ShowEditorGameWindow(bool* isShow);
		void ShowEditorFileContentWindow(bool* isShow);
		void ShowEditorDetailWindow(bool* isShow);

	private:
		bool m_EditorMenuWindowOpen = true;
		bool m_AssetWindowOpen = true;
		bool m_GameEngineWindowOpen = true;
		bool m_FileContentWindowOpen = true;
		bool m_DetailWindowOpen = true;
		bool m_SceneLightsWindowOpen = true;
		bool m_SceneLightsDataWindowOpen = true;
	};

}