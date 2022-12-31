#include "stdafx.h"
#include "RevEngineManager.h"
#include "imgui/imgui.h"
//todo johlander hack because of console
#include "RevEngineMain.h"
#include "RevConsole.h"
#include "RevInputManager.h"
#include "RevRenderManager.h"
#include "RevUIManager.h"

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT ImGui_ImplDX12_WndProcHandler(HWND, UINT msg, WPARAM wParam, LPARAM lParam)
{
	ImGuiIO& io = ImGui::GetIO();
	switch (msg)
	{
	case WM_LBUTTONDOWN:
		io.MouseDown[0] = true;
		return true;
	case WM_LBUTTONUP:
		io.MouseDown[0] = false;
		return true;
	case WM_RBUTTONDOWN:
		io.MouseDown[1] = true;
		return true;
	case WM_RBUTTONUP:
		io.MouseDown[1] = false;
		return true;
	case WM_MOUSEWHEEL:
		io.MouseWheel += GET_WHEEL_DELTA_WPARAM(wParam) > 0 ? +1.0f : -1.0f;
		return true;
	case WM_MOUSEMOVE:
		io.MousePos.x = (signed short)(lParam);
		io.MousePos.y = (signed short)(lParam >> 16);
		return true;
	case WM_KEYDOWN:
		if (wParam < 256)
			io.KeysDown[wParam] = 1;
		switch (wParam)
		{
		case 192:
		{
			//toggle the visibility
			bool consoleNewActiveState = !RevEngineMain::s_instance->m_mainManger->m_console->m_visible;
			RevInputManager::RegisterBlockingContext(EYInputBlockingContext::Console, consoleNewActiveState);
			RevEngineMain::s_instance->m_mainManger->m_console->m_visible = consoleNewActiveState;
		}
		break;
		default:
			break;
		};
		return true;
	case WM_KEYUP:
		if (wParam < 256)
			io.KeysDown[wParam] = 0;
		return true;
	case WM_CHAR:
		// You can also use ToAscii()+GetKeyboardState() to retrieve characters.
		if (wParam > 0 && wParam < 0x10000)
			io.AddInputCharacter((unsigned short)wParam);
		return true;
	}
	return 0;
}

void RevEngineManager::Initialize()
{
	//currently does nothing
	m_console = new RevConsole();
	m_uiManager = new RevUIManager();
	m_uiManager->Initialize(RevEngineMain::s_instance->m_windowHandle);
}


void RevEngineManager::Update(float deltaTime, void* hwnd)
{
	ImGuiIO& io = ImGui::GetIO();
	// Setup display size (every frame to accommodate for window resizing)
	RECT rect;
	GetClientRect((HWND)hwnd, &rect);
	io.DisplaySize = ImVec2((float)(rect.right - rect.left), (float)(rect.bottom - rect.top));

	// Setup time step
	io.DeltaTime = deltaTime;

	// Read keyboard modifiers inputs
	io.KeyCtrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
	io.KeyShift = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
	io.KeyAlt = (GetKeyState(VK_MENU) & 0x8000) != 0;
	// Hide OS mouse cursor if ImGui is drawing it
	SetCursor(io.MouseDrawCursor ? NULL : LoadCursor(NULL, IDC_ARROW));
	// Start the frame
	if (m_isImguiEnabled)
	{
		ImGui::NewFrame();
		m_console->Update();
	}
	UpdateInternal(deltaTime, hwnd);
	if (m_isImguiEnabled)
	{
		ImGui::End();
	}
}

void RevEngineManager::Draw(float deltaTime)
{
	if(m_isImguiEnabled)
	{
		m_uiManager->CopySRV(RevEngineFunctions::FindRenderManager()->m_heapData->m_resource[0]);
		m_uiManager->Draw();
	}
}

void RevEngineManager::DrawDebugFrame()
{

	RevFrameSnapshotData* snapshotData = RevEngineFunctions::FindEngineRuntimeSnapshotData();
	snapshotData->m_mouseDelta = RevEngineFunctions::GetMouseDelta();
	const ImVec2 window_pos = ImVec2(5.0f, 25.0f);
	const ImVec2 window_pos_pivot = ImVec2(0.0f, 0.0f);
	ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.3f)); // Transparent background
	ImGui::PushFont(RevUIManager::FindFontFromType(RevFontType::DroidSans)->m_smallFont);
	ImFont font = {};
	if (ImGui::Begin("DebugData", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings))
	{
		ImGui::Text("FPS: (%.1f)", snapshotData->m_fps);
		ImGui::Text("Camera Location: X (%.1f) Y (%.1f) Z (%.1f)", snapshotData->m_cameraLocation.X(), snapshotData->m_cameraLocation.Y(), snapshotData->m_cameraLocation.Z());
		ImGui::Text("Camera Delta: X (%.1f) Y (%.1f)", snapshotData->m_mouseDelta.X(), snapshotData->m_mouseDelta.Y());
		ImGui::End();
	}
	ImGui::PopStyleColor();
	ImGui::PopFont();
}

LRESULT RevEngineManager::ManageWindowsMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return ImGui_ImplDX12_WndProcHandler(hwnd, msg, wParam, lParam);
}
