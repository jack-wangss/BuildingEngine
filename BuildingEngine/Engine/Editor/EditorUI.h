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
	};

}