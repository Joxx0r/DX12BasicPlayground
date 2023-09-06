#include "stdafx.h"
#include "RevPhysxWorld.h"
#include "RevPhysicsCreationFunctions.h"

void RevPhysxWorld::Initialize()
{
#if WITH_NVIDIA_PHSYX
	//foundation & pvd
	m_physicsManager.m_foundation = PxCreateFoundation(PX_PHYSICS_VERSION, m_physicsManager.m_allocator, m_physicsManager.m_errorCallback);
	m_physicsManager.m_pvd = PxCreatePvd(*m_physicsManager.m_foundation);
	PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate(PVD_HOST, 5425, 10);
	m_physicsManager.m_pvd->connect(*transport,PxPvdInstrumentationFlag::eALL);

	//physics creation
	m_physicsManager.m_physics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_physicsManager.m_foundation, PxTolerancesScale(),true,m_physicsManager.m_pvd);

	//scene creation
	PxSceneDesc sceneDesc(m_physicsManager.m_physics->getTolerancesScale());
	sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
	m_physicsManager.m_dispatcher = PxDefaultCpuDispatcherCreate(2);
	sceneDesc.cpuDispatcher	= m_physicsManager.m_dispatcher;
	sceneDesc.filterShader	= PxDefaultSimulationFilterShader;
	m_physicsManager.m_scene = m_physicsManager.m_physics->createScene(sceneDesc);

	//pvd additional
	PxPvdSceneClient* pvdClient = m_physicsManager.m_scene->getScenePvdClient();
	if(pvdClient)
	{
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
	}
	
	m_physicsManager.m_material = m_physicsManager.m_physics->createMaterial(0.5f, 0.5f, 0.6f);
#endif
	
}

void RevPhysxWorld::Update(float InDeltaTime)
{
	m_physicsManager.m_scene->simulate(InDeltaTime);
	m_physicsManager.m_scene->fetchResults(true);
}

