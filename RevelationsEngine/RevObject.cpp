#include "stdafx.h"
#include "RevObject.h"
#include "RevInstance.h"

RevObject::RevObject()
{
    m_instance = nullptr;
    m_world = nullptr;
}

void RevObject::Initialize(RevWorld* InWorld, const RevMatrix& transform, uint32_t index, const char* modelPath)
{
    REV_ASSERT(InWorld != nullptr);
    m_world = InWorld;
    REV_ASSERT(m_instance == nullptr);
    m_instance = new RevInstance();
    m_instance->Initialize(transform, index, modelPath);
}

void RevObject::Update(struct RevFrameResource* InResource, float InDeltaTime)
{
    m_instance->Update(InResource, InDeltaTime);
}
void RevObject::Draw(RevModelFrameRender& param)
{
    m_instance->Draw(param);
}
