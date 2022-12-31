#include "stdafx.h"
#include "RevEngineGameManager.h"

#include "RevEngineMain.h"
#include "RevWorldLoader.h"
#include "imgui/imgui.h"

std::string GSelectedInstance2;

void RevEngineGameManager::UpdateInternal(float deltaTime, void* hwnd)
{
	DrawDebugFrame();
	RevFrameSnapshotData* snapshotData = RevEngineFunctions::FindEngineRuntimeSnapshotData();
	snapshotData->m_mouseDelta = RevEngineFunctions::GetMouseDelta();
}
