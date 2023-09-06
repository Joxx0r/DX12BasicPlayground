#pragma once

#include "RevPhysxTypes.h"

namespace physx
{
	class PxRigidStatic;
	class PxScene;
	class PxMaterial;
	class PxPhysics;
	class PxRigidDynamic;
	class PxGeometry;
}

class RevPhysicsCreationFunctions
{
public:
#if WITH_NVIDIA_PHSYX
	static void createDynamic(PxScene* InScene, PxPhysics* InPhysics, PxMaterial* InMaterial, const PxTransform& t, const PxGeometry& geometry, const PxVec3& velocity);
	static void createStack(PxPhysics* InPhysics, PxScene* InScene, physx::PxMaterial* InMaterial, const PxTransform& t, PxU32 size, PxReal halfExtent);
	static void CreatePlane(PxPhysics* InPhysics, PxScene* InScene, PxMaterial* InMaterial);
#endif
};