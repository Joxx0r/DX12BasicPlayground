#include "stdafx.h"
#include "RevInstance.h"
#include "RevModelManager.h"
#include "RevFrameResource.h"
#include "RevModel.h"
#include "RevAnimation.h"
#include "RevTypes.h"

float GAnimationRateScale = 1.0f;

RevInstance::RevInstance()
{
	m_world.Identity();
	m_deltaTime = 0.0f;
}

void RevInstance::Initialize(const RevMatrix& transform, UINT index, const char* modelPath)
{
	m_cbufferIndex = index;
	m_world = transform;
	m_modelHandle = RevModelManager::FindModel(modelPath)->m_handle;
	assert(m_modelHandle != UINT_MAX);
}

void RevInstance::Update(struct RevFrameResource* resource, float deltaTime)
{
	m_deltaTime += deltaTime *GAnimationRateScale;
	RevModel* entry = RevModelManager::FindModelByHandle(m_modelHandle);
	assert(entry);
	if (entry->m_type == RevModelType::Animated)
	{
		ObjectConstantsAnimated objConstants;
		objConstants.WorldViewProj = m_world.Transpose();

		RevAnimationUpdateData updateData(&objConstants.m_bones[0],entry->m_modelData->m_bones, m_deltaTime);
		RevEngineFunctions::RequestAnimationUpdate(updateData, entry->m_modelData->m_animationInstances[0]);
		for (UINT index = 0; index < ARRAYSIZE(objConstants.m_bones); index++)
		{
			objConstants.m_bones[index] = objConstants.m_bones[index].Transpose();
		}
		resource->m_animatedObjectCB->CopyData(m_cbufferIndex, objConstants);
	}
	else
	{
		ObjectConstants objConstants;
		objConstants.WorldViewProj = m_world.Transpose();
		resource->m_objectCB->CopyData(m_cbufferIndex, objConstants);

	}
}

void RevInstance::Draw(RevModelFrameRender& param)
{
	param.m_startIndex = m_cbufferIndex;
	RevModel* entry = RevModelManager::FindModelByHandle(m_modelHandle);
	assert(entry);
	entry->Draw(param);
}
