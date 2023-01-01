#include "stdafx.h"
#include "RevController.h"
#include "RevCamera.h"
#include "RevFrameResource.h"
#include "RevMovementFunctions.h"

void RevController::Initialize()
{
	REV_ASSERT(m_camera == nullptr);
	m_camera = new RevCamera();
	const RevVector3 cameraDefaultLocation(0, 10, -4);
	m_camera->Initialize(cameraDefaultLocation, RevVector3::ZeroVector);
	RefreshInitialization();
}

void RevController::SetInstance(RevInstance* inInstance)
{
	m_instance = inInstance;
	RefreshInitialization();
}

void RevController::Update(float deltaTime)
{
	if(m_instance)
	{
		m_instance->UpdateLocationInput(deltaTime, RevMovementFunctions::CalculateCurrentInput());
		RefreshInitialization();
		m_camera->Update(deltaTime);
	}
}

void RevController::Draw(float deltaTime, struct RevModelFrameRender& renderEntry)
{
	REV_ASSERT(m_camera);
	RevFrameResource* resource = RevEngineFunctions::FindFrameResource(renderEntry.m_currentRenderFrameResourceIndex);
	m_camera->UpdateRendererData(resource->m_passCB);
}

void RevController::RefreshInitialization()
{
	if(m_camera != nullptr && m_instance != nullptr)
	{
		RevVector3 cameraDefaultLocation = RevVector3(0, 10, -15) + m_instance->m_transform.GetLocation();
		m_camera->Initialize(cameraDefaultLocation, m_instance->m_transform.GetLocation());
	}
}