#include "stdafx.h"
#include "RevGameState.h"
#include "RevObject.h"
#include "RevInstance.h"

void RevGameState::AddGameObject(RevObject* InObject)
{
    REV_ASSERT(InObject != nullptr);
    m_gameObjects.push_back(InObject);
}
void RevGameState::Initialize()
{
    m_gameObjects.reserve(1000);
}

void RevGameState::Update(RevFrameResource* InResource, float InDeltaTime)
{
    for (RevObject* gameObject : m_gameObjects)
    {
        gameObject->Update(InResource, InDeltaTime);
    }
}

void RevGameState::Draw(RevModelFrameRender& param)
{
    for (RevObject* gameObject : m_gameObjects)
    {
        gameObject->Draw(param);
    }
}

bool RevGameState::ReplaceObject(UINT index, RevObject* newObject)
{
    if (index < 0)
    {
        REV_ASSERT(false);
        return false;
    }

    if (index >= 0 && index <= m_gameObjects.size())
    {
        delete m_gameObjects[index];
        newObject->m_instance->m_cbufferIndex = index;
        m_gameObjects[index] = newObject;
    }
    else
    {
        newObject->m_instance->m_cbufferIndex = (UINT)m_gameObjects.size();
        m_gameObjects.push_back(newObject);
    }
    return true;
}
void RevGameState::Destroy()
{
    for (UINT instanceIndex = 0; instanceIndex < m_gameObjects.size(); instanceIndex++)
    {
        delete m_gameObjects[instanceIndex];
    }
    m_gameObjects.clear();
}
