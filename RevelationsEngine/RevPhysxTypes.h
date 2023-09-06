#pragma once

//todo johlander resolve dynamic physx include
#include "PxPhysicsAPI.h"

using namespace physx;

struct RevPhysxInstance
{
    PxDefaultAllocator		m_allocator;
    PxDefaultErrorCallback	m_errorCallback;
    PxFoundation* m_foundation = NULL;
    PxPhysics* m_physics = NULL;
    PxDefaultCpuDispatcher* m_dispatcher = NULL;
    PxScene* m_scene = NULL;
    PxMaterial* m_material = NULL;
    PxPvd* m_pvd = NULL;
};

#define PVD_HOST "127.0.0.1"	//Set this to the IP address of the system running the PhysX Visual Debugger that you want to connect to.
