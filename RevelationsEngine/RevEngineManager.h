#pragma once

class RevEngineManager
{
public:
	RevEngineManager() { }

	virtual void Initialize();
	void Update(float deltaTime, void* hwnd);

	void DrawDebugFrame();

	static LRESULT ManageWindowsMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

protected:

	//Internal implementation from derived classes
	virtual void UpdateInternal(float deltaTime, void* hwnd) {};

public:

	class RevConsole* m_console = nullptr;

protected:

	bool m_isImguiEnabled = true;
};