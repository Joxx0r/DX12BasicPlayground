#include "stdafx.h"
#include "RevWorld.h"

void RevWorld::DestoyWorld()
{
	for (UINT instanceIndex = 0; instanceIndex < m_instances.size(); instanceIndex++)
	{
		delete m_instances[instanceIndex];
	}
	m_instances.clear();
}

void RevWorld::Draw(RevModelFrameRender& render)
{
	for (UINT instanceIndex = 0; instanceIndex < m_instances.size(); instanceIndex++)
	{
		m_instances[instanceIndex]->Draw(render);
	}
}

void RevWorld::UpdateRendererData(struct RevFrameResource* resource, float deltaTime)
{
	assert(m_instances.size() < AMOUNT_OF_MAX_INSTANCES);
	for (INT32 index = 0; index < m_instances.size(); index++)
	{
		m_instances[index]->Update(resource, deltaTime);
	}
}

void RevWorld::ReplaceInstance(UINT index, RevInstance* newInstance)
{
	if (index < 0)
	{
		//log error
		return;
	}

	if (index >= 0 && index <= m_instances.size())
	{
		delete m_instances[index];
		newInstance->m_cbufferIndex = index;
		m_instances[index] = newInstance;
	}
	else
	{
		newInstance->m_cbufferIndex = (UINT)m_instances.size();
		m_instances.push_back(newInstance);
	}
}
