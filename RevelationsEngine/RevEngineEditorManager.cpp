#include "stdafx.h"
#include "RevEngineEditorManager.h"
#include "RevEngineMain.h"
#include "RevWorldLoader.h"
#include "imgui/imgui.h"

bool GUsePropertyEditor = true;
std::string GSelectedInstance;

void RevEngineEditorManager::Initialize()
{
	RevEngineManager::Initialize();
}

void RevEngineEditorManager::UpdateInternal(float deltaTime, void* hwnd)
{
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
				sprintf(label, "Model%s", objectType.m_name.c_str());

				ImGui::Bullet();
				ImGui::Selectable(label);
				if (ImGui::IsItemActive())
				{
					if (GSelectedInstance.c_str() != objectType.m_name)
					{
						GSelectedInstance = objectType.m_name;
						RevEngineMain::s_instance->m_worldLoader->SpawnInstanceToWorld(
							RevEngineMain::s_instance->m_activeWorld,
							objectType.m_name.c_str());
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
					pathToUse = pathToUse.substr(0, lastPath2);
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
}