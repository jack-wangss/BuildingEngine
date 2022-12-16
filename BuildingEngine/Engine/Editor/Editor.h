#pragma once

namespace BE
{
	class EditorUI;
	class Engine;

	class Editor
	{
	public:
		Editor();
		virtual ~Editor();
	public:
		void Initialize(Engine* engine);
		void Clear();
		void Run();


	protected:
		std::shared_ptr<EditorUI> m_UI;
		Engine* m_Engine{nullptr};
	};
}