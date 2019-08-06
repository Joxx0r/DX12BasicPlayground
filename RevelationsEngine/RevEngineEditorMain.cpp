#include "stdafx.h"
#include "RevEngineEditorMain.h"
#include "imgui/imgui.h"
//todo johlander hack because of console
#include "RevEngineMain.h"

#include "RevConsole.h"
#include "RevWorldLoader.h"

bool GUsePropertyEditor = true;

std::string GSelectedInstance;

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
				extern bool GBlockGameInput;
				GBlockGameInput = !GBlockGameInput;
				RevEngineMain::s_instance->m_editor->m_console->m_visible = !RevEngineMain::s_instance->m_editor->m_console->m_visible;
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

void RevEngineEditorMain::Initialize()
{
	//currently does nothing
	m_console = new RevConsole();
}


void RevEngineEditorMain::Update(float deltaTime, void* hwnd)
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
	ImGui::NewFrame();

	
	static float f = 0.0f;
	
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Open Level"))
			{
				//when we can load full level xml
			}

			if (ImGui::MenuItem("Save Model"))
			{
				RevEngineFunctions::RequestEditorAction(RevEditorActions::SaveModel);
			}

			ImGui::Separator();

			if (ImGui::MenuItem("Quit"))
			{
				//todo johlander clearner handling later.
				extern bool GApplicationIsRunning;
				GApplicationIsRunning = false;
			}

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Options"))
		{
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}
	
	DrawDebugFrame();

	if (GUsePropertyEditor)
	{
		const float DISTANCE = 85.0f;
		ImVec2 window_pos = ImVec2(5.0f, DISTANCE);

		ImVec2 window_pos_pivot = ImVec2(0.0f, 0.0f);
		ImGui::SetNextWindowPos(window_pos, ImGuiCond_FirstUseEver, window_pos_pivot);
		ImGui::SetNextWindowSize(ImVec2(350, 500), ImGuiCond_FirstUseEver);
		ImGui::Begin("Example: Property editor", nullptr);

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
		ImGui::Columns(2);
		ImGui::Separator();

		ImGui::PushID(0);
		ImGui::AlignTextToFramePadding();
		bool node_open = ImGui::TreeNode("Models");
		ImGui::NextColumn();
		ImGui::AlignTextToFramePadding();
		ImGui::Text("All model files");
		ImGui::NextColumn();
		if (node_open)
		{
			INT32 index = 1;
			const std::vector< RevGameObjectType>& objectTypes = RevEngineMain::s_instance->m_worldLoader->m_objectTypes;
			for (const RevGameObjectType& objectType : objectTypes)
			{
				ImGui::PushID(index);
				ImGui::AlignTextToFramePadding();
				char label[512];
				sprintf(label, "Model%s", objectType.m_name);
				
		
				ImGui::Bullet();
				ImGui::Selectable(label);
				if (ImGui::IsItemActive())
				{
					if (GSelectedInstance.c_str() != objectType.m_name)
					{
						GSelectedInstance = objectType.m_name;
						RevEngineMain::s_instance->m_worldLoader->SpawnInstanceToWorld(
							RevEngineMain::s_instance->m_activeWorld,
							objectType.m_name);
					}
				}
				ImGui::NextColumn();
				ImGui::PushItemWidth(-1);
				index++;
				ImGui::PopItemWidth();


				ImGui::AlignTextToFramePadding();
				std::string fullPath = objectType.m_instancePath;

				size_t lastPath = fullPath.find_last_of('\\') + 1;
				std::string pathToUse = fullPath.substr(lastPath, fullPath.length());

				{
					size_t lastPath2 = pathToUse.find_last_of('.');
					pathToUse = pathToUse.substr(0, lastPath2 );
				}

				sprintf(label, "%s", fullPath.c_str());
				ImGui::Text(label);
				
				ImGui::NextColumn();
				ImGui::PopID();
			}

			ImGui::TreePop();
		}

		ImGui::PopID();
		
		ImGui::Columns(1);
		ImGui::Separator();
		ImGui::PopStyleVar();

		RevFrameSnapshotData* snapshotData = RevEngineFunctions::FindEngineRuntimeSnapshotData();
		snapshotData->m_mouseDelta = RevEngineFunctions::GetMouseDelta();

		ImGui::End();

	}
	m_console->Update();
}

void RevEngineEditorMain::DrawDebugFrame()
{
	{

		RevFrameSnapshotData* snapshotData = RevEngineFunctions::FindEngineRuntimeSnapshotData();
		snapshotData->m_mouseDelta = RevEngineFunctions::GetMouseDelta();
		RevVector2 deltaMove = RevEngineFunctions::GetMouseDelta();
		const float DISTANCE = 25.0f;
		static int corner = 0;
		ImVec2 window_pos = ImVec2(5.0f, DISTANCE);
		ImVec2 window_pos_pivot = ImVec2((corner & 1) ? 1.0f : 0.0f, (corner & 2) ? 1.0f : 0.0f);
		ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.3f)); // Transparent background
		if (ImGui::Begin("DebugData", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings))
		{
			ImGui::Text("FPS: (%.1f)", snapshotData->m_fps);
			ImGui::Text("Camera Location: X (%.1f) Y (%.1f) Z (%.1f)", snapshotData->m_cameraLocation.X(), snapshotData->m_cameraLocation.Y(), snapshotData->m_cameraLocation.Z());
			ImGui::Text("Camera Delta: X (%.1f) Y (%.1f)", snapshotData->m_mouseDelta.X(), snapshotData->m_mouseDelta.Y());

			ImGui::End();
		}
		ImGui::PopStyleColor();
	}
}

LRESULT RevEngineEditorMain::ManageWindowsMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return ImGui_ImplDX12_WndProcHandler(hwnd, msg, wParam, lParam);
}
