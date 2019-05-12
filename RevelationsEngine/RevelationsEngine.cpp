// RevelationsEngine.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "RevelationsEngine.h"
#include "RevEngineMain.h"
#include <d3d12.h>

#define MAX_LOADSTRING 100

bool GApplicationIsRunning = false;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code that uses hdc here...
		EndPaint(hWnd, &ps);
	}
	break;
	case WM_DESTROY:
		PostQuitMessage(0);
		GApplicationIsRunning = false;
		break;
	default:
	{
		if (!RevEngineMain::ManageWindowsMessage(hWnd, message, wParam, lParam))
		{
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	};
	}
	return 0;
}


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

	RevInitializationData initializationData = {};
	initializationData.m_windowHandle = WndProc;
	initializationData.m_instance = hInstance;
	initializationData.m_windowWidth = 1920;
	initializationData.m_windowHeight = 1080;
	RevEngineMain::Create(initializationData);
    
	MSG msg = {};
	GApplicationIsRunning = true;
	LARGE_INTEGER frequency, start, end;

	QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&start);
	float targetFrameRate = 60.0f;
	float invFrameRate = 1 / targetFrameRate;
	while (GApplicationIsRunning)
	{
		QueryPerformanceCounter(&end);
		float t = ((float)end.QuadPart - (float)start.QuadPart) / (float)frequency.QuadPart;
		start = end;

		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}


		RevEngineMain::Update(t);
		RevEngineMain::Draw(t);

		if ((GetKeyState(VK_ESCAPE) >> 15) != 0)
		{
			GApplicationIsRunning = false;
		}
		
		
	}
	
    return 0;
}
