#pragma once

class RevEngineEditorMain
{
public:
	RevEngineEditorMain()
	{
	}

	void Initialize();
	void Update(float deltaTime, void* hwnd);

	void DrawDebugFrame();

	static LRESULT ManageWindowsMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	class RevConsole* m_console = nullptr;
};