#include "stdafx.h"
#include "RevController.h"
#include "RevCamera.h"
#include "RevFrameResource.h"

void RevController::Initialize()
{
	REV_ASSERT(m_camera == nullptr);
	m_camera = new RevCamera();
	const RevVector cameraDefaultLocation(0, 1, 4);
	m_camera->Initialize(cameraDefaultLocation);

}

void RevController::Update(float deltaTime)
{
	REV_ASSERT(m_camera);
	m_camera->UpdateLocation(deltaTime);
}

void RevController::Draw(float deltaTime, struct RevModelFrameRender& renderEntry)
{
	REV_ASSERT(m_camera);
	RevFrameResource* resource = RevEngineFunctions::FindFrameResource(renderEntry.m_currentRenderFrameResourceIndex);
	m_camera->UpdateRendererData(resource->m_passCB);
}
