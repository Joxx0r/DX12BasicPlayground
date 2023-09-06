#include "stdafx.h"
#include "RevWorld.h"
#include "RevGameState.h"
#include "RevPhysxWorld.h"

RevWorld::RevWorld()
{
	m_revGameState = nullptr;
	m_physicsWorld = nullptr;
}

void RevWorld::Initialize()
{
	m_physicsWorld = new RevPhysxWorld();
	m_physicsWorld->Initialize();
	
	m_revGameState = new RevGameState();
	m_revGameState->Initialize();
}

void RevWorld::Draw(RevModelFrameRender& render)
{
	m_revGameState->Draw(render);
}

void RevWorld::UpdateRendererData(struct RevFrameResource* resource, float deltaTime)
{
	REV_ASSERT(m_revGameState->m_gameObjects.size() < AMOUNT_OF_MAX_INSTANCES);
	m_physicsWorld->Update(deltaTime);
	m_revGameState->Update(resource, deltaTime);
}

void RevWorld::DestoyWorld()
{
	m_revGameState->Destroy();
	delete m_revGameState;
	m_revGameState = nullptr;
}

