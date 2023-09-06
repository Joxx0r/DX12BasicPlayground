#include "stdafx.h"
#include "RevEngineGameManager.h"

std::string GSelectedInstance2;

void RevEngineGameManager::UpdateInternal(float deltaTime, void* hwnd)
{
	DrawDebugFrame();

	RevFrameSnapshotData* snapshotData = RevEngineFunctions::FindEngineRuntimeSnapshotData();
	snapshotData->m_mouseDelta = RevEngineFunctions::GetMouseDelta();
}
