#pragma once

#include "RevEngineManager.h"

class RevEngineGameManager : public RevEngineManager
{
public:
	RevEngineGameManager()
	{
		m_isImguiEnabled = true;
	}

protected:

	virtual void UpdateInternal(float deltaTime, void* hwnd);
};
