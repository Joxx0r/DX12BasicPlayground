#include "stdafx.h"
#include "RevPhysicsCreationFunctions.h"
#include "PxPhysicsAPI.h"

void RevPhysicsCreationFunctions::createDynamic(physx::PxScene* InScene, physx::PxPhysics* InPhysics, physx::PxMaterial* InMaterial, const physx::PxTransform& t, const physx::PxGeometry& geometry, const physx::PxVec3& velocity)
{
    REV_ASSERT(InScene != nullptr || InPhysics != nullptr  ||  InMaterial != nullptr );
    PxRigidDynamic* dynamic = PxCreateDynamic(*InPhysics, t, geometry, *InMaterial, 10.0f);
    dynamic->setAngularDamping(0.5f);
    dynamic->setLinearVelocity(velocity);
    InScene->addActor(*dynamic);
}

void RevPhysicsCreationFunctions::createStack(PxPhysics* InPhysics, PxScene* InScene, physx::PxMaterial* InMaterial, const PxTransform& t, PxU32 size, PxReal halfExtent)
{
    PxShape* shape = InPhysics->createShape(PxBoxGeometry(halfExtent, halfExtent, halfExtent), *InMaterial);
    for (PxU32 i = 0; i < size; i++)
    {
        for (PxU32 j = 0; j < size - i; j++)
        {
            PxTransform localTm(PxVec3(PxReal(j * 2) - PxReal(size - i), PxReal(i * 2 + 1), 0) * halfExtent);
            PxRigidDynamic* body = InPhysics->createRigidDynamic(t.transform(localTm));
            body->attachShape(*shape);
            PxRigidBodyExt::updateMassAndInertia(*body, 10.0f);
            InScene->addActor(*body);
        }
    }
    shape->release();
}
void RevPhysicsCreationFunctions::CreatePlane(PxPhysics* InPhysics, PxScene* InScene, PxMaterial* InMaterial)
{
    PxRigidStatic* createdPlane =  PxCreatePlane(*InPhysics, PxPlane(0,1,0,0), *InMaterial);
    InScene->addActor(*createdPlane);    
}
