#pragma once

#include "RevPhysxTypes.h"

class RevPhysxWorld
{
public:
    void Initialize();
    void Update(float InDeltaTime);
    
    RevPhysxInstance m_physicsManager;
};