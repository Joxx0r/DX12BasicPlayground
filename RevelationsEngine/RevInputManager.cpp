#include "stdafx.h"
#include "RevInputManager.h"

bool RevInputManager::IsInputEnabled(EYInputType desiredType)
{
    if(RevInputManager* manager = RevEngineFunctions::FindInputManager())
    {
        if(manager->FindBlockingContext(EYInputBlockingContext::WindowApplication) != REV_INDEX_NONE)
        {
            return false;
        }
        if(desiredType == EYInputType::Game)
        {
            return !(manager->m_blockingContexts.size() > 0);
        }
    }
    
    return true;
}

bool RevInputManager::RegisterBlockingContext(EYInputBlockingContext context, bool state)
{
    if(RevInputManager* manager = RevEngineFunctions::FindInputManager())
    {
        for (int index =0; index < manager->m_blockingContexts.size(); index++)
        {
            EYInputBlockingContext blockingContext = manager->m_blockingContexts[index];
            if(context == blockingContext)
            {
                if(state)
                {
                    return true;
                }
                if(!state)
                {
                    manager->m_blockingContexts.erase(manager->m_blockingContexts.begin() + index);
                    return true;
                }
            }
        }
        if(state)
        {
            manager->m_blockingContexts.push_back(context);
            return true;
        }
    }
    return false;
}

bool RevInputManager::IsContextBlocked(EYInputBlockingContext context)
{
    if(RevInputManager* manager = RevEngineFunctions::FindInputManager())
    {
        return manager->FindBlockingContext(context) != REV_INDEX_NONE;
    }
    return false;
}

int RevInputManager::FindBlockingContext(EYInputBlockingContext context)
{
    for (uint32_t index =0; index < m_blockingContexts.size(); index++)
    {
        EYInputBlockingContext blockingContext = m_blockingContexts[index];
        if(context == blockingContext)
        {
            return static_cast<int32_t>(index);
        }
    }
    return -1;
}
