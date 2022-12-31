#pragma once

#include "RevEngineManager.h"

class RevEngineGameManager : public RevEngineManager
{
public:
	RevEngineGameManager()
	{
	}

protected:

	virtual void UpdateInternal(float deltaTime, void* hwnd);
};
